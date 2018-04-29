#
#  The OpenDiamond Platform for Interactive Search
#
#  Copyright (c) 2011 Carnegie Mellon University
#  All rights reserved.
#
#  This software is distributed under the terms of the Eclipse Public
#  License, Version 1.0 which can be found in the file named LICENSE.
#  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
#  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
#

'''Statistics tracking.'''

from __future__ import with_statement
import logging
import threading
import time

from opendiamond.protocol import XDR_search_stats, XDR_filter_stats, XDR_stat

_log = logging.getLogger(__name__)


class FilterRunnerLogger(object):

    def __init__(self, stats):
        assert isinstance(stats, FilterStatistics)
        self.stats = stats
        self.eval_timer = Timer()

    def on_connected(self):
        pass

    def on_initialized(self):
        pass

    def on_start_evaluate(self):
        self.eval_timer.reset()

    def on_done_evaluate(self, accept):
        with self.stats.lock:
            self.stats.execution_us += self.eval_timer.elapsed
            self.stats.objs_processed += 1
            self.stats.objs_computed += 1
            self.stats.objs_dropped += int(not accept)

    def on_cache_hit(self, accept):
        with self.stats.lock:
            self.stats.objs_processed += 1
            self.stats.objs_dropped += int(not accept)
            self.stats.objs_cache_dropped += int(not accept)
            self.stats.objs_cache_passed += int(accept)

    def on_terminate(self):
        with self.stats.lock:
            self.stats.objs_terminate += 1


class FilterStackRunnerLogger(object):

    def __init__(self, stats):
        assert isinstance(stats, SearchStatistics)
        self.stats = stats
        self.eval_timer = Timer()

    def on_start_evaluate(self):
        self.eval_timer.reset()

    def on_done_evaluate(self, accept):
        with self.stats.lock:
            self.stats.objs_processed += 1
            self.stats.objs_passed += int(accept)
            self.stats.objs_dropped += int(not accept)
            self.stats.execution_us += self.eval_timer.elapsed

    def on_unloadable(self):
        with self.stats.lock:
            self.stats.objs_unloadable += 1


class _Statistics(object):
    '''Base class for server statistics.'''

    label = 'Unconfigured statistics'
    attrs = ()

    def __init__(self):
        self.lock = threading.Lock()
        self._stats = dict([(name, 0) for name, _desc in self.attrs])

    def __getattr__(self, key):
        return self._stats[key]

    def log(self):
        """Dump all statistics to the log."""
        _log.info('%s:', self.label)
        with self.lock:
            for name, desc in self.attrs:
                _log.info('  %s: %d', desc, getattr(self, name))


class SearchStatistics(_Statistics):
    label = 'Search statistics'
    attrs = (('objs_processed', 'Objects considered'),
             ('objs_dropped', 'Objects dropped'),
             ('objs_passed', 'Objects passed'),
             ('objs_unloadable', 'Objects failing to load'),
             ('execution_us', 'Total object examination time (us)'))

    def __init__(self):
        super(SearchStatistics, self).__init__()

    def xdr(self, objs_total, filter_stats):
        '''Return an XDR statistics structure for these statistics.'''
        with self.lock:
            try:
                avg_obj_us = self.execution_us / self.objs_processed
            except ZeroDivisionError:
                avg_obj_us = 0

            stats = []
            stats.append(XDR_stat('objs_total', objs_total))
            stats.append(XDR_stat('avg_obj_time_us', avg_obj_us))
            for name, _desc in self.attrs:
                if name != 'execution_us':
                    stats.append(XDR_stat(name, getattr(self, name)))

            return XDR_search_stats(
                stats=stats,
                filter_stats=[s.xdr() for s in filter_stats],
            )


class FilterStatistics(_Statistics):
    '''Statistics for the execution of a single filter.'''

    attrs = (('objs_processed', 'Total objects considered'),
             ('objs_dropped', 'Total objects dropped'),
             ('objs_cache_dropped', 'Objects dropped by cache'),
             ('objs_cache_passed', 'Objects skipped by cache'),
             ('objs_computed', 'Objects examined by filter'),
             ('objs_terminate', 'Objects causing filter to terminate'),
             ('execution_us', 'Filter execution time (us)'),
             )

    def __init__(self, name):
        _Statistics.__init__(self)
        self.name = name
        self.label = 'Filter statistics for %s' % name

    def xdr(self):
        '''Return an XDR statistics structure for these statistics.'''
        with self.lock:
            try:
                avg_exec_us = self.execution_us / self.objs_processed
            except ZeroDivisionError:
                avg_exec_us = 0

            stats = []
            stats.append(XDR_stat('avg_exec_time_us', avg_exec_us))
            for name, _desc in self.attrs:
                if name != 'execution_us':
                    stats.append(XDR_stat(name, getattr(self, name)))

            return XDR_filter_stats(
                name=self.name,
                stats=stats
            )


class Timer(object):
    '''Tracks the elapsed time since the Timer object was created.'''

    def __init__(self):
        self._start = time.time()

    @property
    def elapsed_seconds(self):
        '''Elapsed time in seconds.'''
        return time.time() - self._start

    @property
    def elapsed(self):
        '''Elapsed time in us.'''
        return int(self.elapsed_seconds * 1e6)

    def reset(self):
        self._start = time.time()
