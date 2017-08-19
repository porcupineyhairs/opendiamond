import json
import os
import datetime
from flask import Blueprint, url_for, Response, stream_with_context, send_file, \
    request
from math import ceil
from werkzeug.datastructures import Headers

from opendiamond.dataretriever.util import DiamondTextAttr
import subprocess

BASEURL = 'atomizer'
STYLE = False
INDEXDIR = DATAROOT = None


def init(config):
    global INDEXDIR, DATAROOT  # pylint: disable=global-statement
    INDEXDIR = config.indexdir
    DATAROOT = config.dataroot


scope_blueprint = Blueprint('atomizer_store', __name__)


@scope_blueprint.route('/<int:stride_sec>/<int:span_sec>/<gididx>')
def get_scope(stride_sec, span_sec, gididx):
    index = 'GIDIDX' + gididx.upper()
    index = os.path.join(INDEXDIR, index)

    def generate():
        yield '<?xml version="1.0" encoding="UTF-8" ?>\n'
        if STYLE:
            yield '<?xml-stylesheet type="text/xsl" href="/scopelist.xsl" ?>\n'

        yield '<objectlist>\n'

        with open(index, 'r') as f:
            for video in f:
                video = video.strip()
                video_path = str(os.path.join(DATAROOT, video))
                meta = _maybe_parse_video_and_get_attrs(video_path=video_path)
                num_clips = int(ceil(float(meta['duration_sec']) / stride_sec))
                yield '<count adjust="{}"/>\n'.format(num_clips)
                for clip in range(num_clips):
                    obj_url = url_for('.get_obj',
                                      start_sec=clip * stride_sec,
                                      span_sec=span_sec,
                                      video=video)
                    yield '<object src="{}" />\n'.format(obj_url)

        yield '</objectlist>\n'

    headers = Headers([('Content-Type', 'text/xml')])
    return Response(stream_with_context(generate()),
                    status="200 OK",
                    headers=headers)


@scope_blueprint.route('/obj/<int:start_sec>/<int:span_sec>/<path:video>')
def get_obj(start_sec, span_sec, video):
    # Reference:
    # https://github.com/mikeboers/PyAV/blob/master/tests/test_seek.py
    video_path = str(os.path.join(DATAROOT, video))
    proc = _create_ffmpeg_segment_proc(video_path,
                                       start_sec=start_sec,
                                       duration_sec=span_sec)

    def generate():
        while True:
            data = proc.stdout.read(4096)
            if not data:
                break
            yield data

    headers = Headers([('Content-Type', 'video/mp4')])
    response = Response(stream_with_context(generate()),
                        status="200 OK",
                        headers=headers)
    # Cache control
    stat = os.stat(video_path)
    last_modified = stat.st_mtime
    size = stat.st_size
    etag = "{}_{}_{}_{}".format(last_modified, size, start_sec, span_sec)
    response.last_modified = last_modified
    response.set_etag(etag=etag)
    response.cache_control.public = True
    response.cache_control.max_age = \
        datetime.timedelta(days=365).total_seconds()
    response.make_conditional(request)

    return response


def _maybe_parse_video_and_get_attrs(video_path):
    """
    ffprobe a video and write meta data to text attr file if not exists.
    Return the text attr as a dict.
    Reference:
    https://trac.ffmpeg.org/wiki/FFprobeTips
    """
    # FIXME maybe compare mtime and overwrite stale attr if necessary?
    if not DiamondTextAttr.exists(video_path):
        cmd_l = ['ffprobe', '-v', 'quiet', '-print_format', 'json',
                 '-show_format', video_path]
        proc = subprocess.Popen(cmd_l, stdout=subprocess.PIPE, bufsize=-1)
        meta = json.load(proc.stdout)

        with DiamondTextAttr(video_path, 'w') as attrs:
            attrs.write("duration_sec", float(meta['format']['duration']))

    with DiamondTextAttr(video_path, 'r') as attrs:
        rv = dict()
        for k, v in attrs:
            rv[k] = v
        return rv


def _create_ffmpeg_segment_proc(video_path, start_sec, duration_sec):
    """
    Use ffmpeg to extract a .mp4 segment of the video. Outfile is written to stdout.
    Note: recommend ffmpeg >= 3.3.
    Inaccurate segmentation is observed with 2.7.
    Reference: http://trac.ffmpeg.org/wiki/Seeking
    https://stackoverflow.com/questions/34123272/ffmpeg-transmux-mpegts-to-mp4-gives-error-muxer-does-not-support-non-seekable
    :param video_path:
    :param start_sec:
    :param duration_sec:
    :return: the subprocess
    """
    cmd_l = ['ffmpeg', '-v', 'quiet',
             '-ss', str(start_sec),
             '-t', str(duration_sec),
             '-i', str(video_path),
             '-movflags', 'frag_keyframe+empty_moov',
             '-c', 'copy',
             '-f', 'mp4',
             'pipe:1']

    proc = subprocess.Popen(cmd_l, stdout=subprocess.PIPE, bufsize=-1)
    return proc
