/*
 * These file handles a lot of the device specific code.  For the current
 * version we have state for each of the devices.
 */
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include "ring.h"
#include "rstat.h"
#include "lib_searchlet.h"
#include "obj_attr.h"
#include "lib_odisk.h"
#include "filter_priv.h"	/* to read stats -RW */ 
#include "search_state.h"
#include "lib_sstub.h"


typedef enum {
	DEV_STOP,
	DEV_TERM,
	DEV_START,
	DEV_SEARCHLET
} dev_op_type_t;


typedef struct {
	char *filter;
	char *spec;
} dev_slet_data_t;


typedef struct {
	dev_op_type_t	cmd;
	int		id;
	union {
		dev_slet_data_t	sdata;
	} extra_data;
} dev_cmd_data_t;
	


int
search_stop(void *app_cookie, int gen_num)
{
	dev_cmd_data_t *	cmd;
	search_state_t *	sstate;
	int			err;

	sstate = (search_state_t *)app_cookie;

	cmd = (dev_cmd_data_t *) malloc(sizeof(*cmd));	
	if (cmd == NULL) {
		return (1);
	}

	cmd->cmd = DEV_STOP;
	cmd->id = gen_num;

	err = ring_enq(sstate->control_ops, (void *)cmd);
	if (err) {
		free(cmd);
		return (1);
	}
	return (0);
}


int
search_term(void *app_cookie, int id)
{
	dev_cmd_data_t *	cmd;
	search_state_t *	sstate;
	int			err;

	sstate = (search_state_t *)app_cookie;

	/*
	 * Allocate a new command and put it onto the ring
	 * of commands being processed.
	 */
	cmd = (dev_cmd_data_t *) malloc(sizeof(*cmd));	
	if (cmd == NULL) {
		return (1);
	}
	cmd->cmd = DEV_TERM;
	cmd->id = id;

	/*
	 * Put it on the ring.
	 */
	err = ring_enq(sstate->control_ops, (void *)cmd);
	if (err) {
		free(cmd);
		return (1);
	}
	return (0);
}


int
search_start(void *app_cookie, int id)
{
	dev_cmd_data_t *	cmd;
	int			err;
	search_state_t *	sstate;

	/* XXX start */

	printf("search start \n");
	sstate = (search_state_t *)app_cookie;
	cmd = (dev_cmd_data_t *) malloc(sizeof(*cmd));	
	if (cmd == NULL) {
		return (1);
	}

	cmd->cmd = DEV_START;
	cmd->id = id;


	err = ring_enq(sstate->control_ops, (void *)cmd);
	if (err) {
		free(cmd);
		return (1);
	}


	return (0);
}


/*
 * This is called to set the searchlet for the current search.
 */

int
search_set_searchlet(void *app_cookie, int id, char *filter, char *spec)
{
	dev_cmd_data_t *cmd;
	int		err;
	search_state_t * sstate;

	sstate = (search_state_t *)app_cookie;

	cmd = (dev_cmd_data_t *) malloc(sizeof(*cmd));	
	if (cmd == NULL) {
		return (1);
	}

	cmd->cmd = DEV_SEARCHLET;
	cmd->id = id;
	
	cmd->extra_data.sdata.filter = filter;
	cmd->extra_data.sdata.spec = spec;

	err = ring_enq(sstate->control_ops, (void *)cmd);
	if (err) {
		free(cmd);
		return (1);
	}
	return (0);
}



/*
 * Take the current command and process it.  Note, the command
 * will be free'd by the caller.
 */
static void
dev_process_cmd(search_state_t *sstate, dev_cmd_data_t *cmd)
{
	int	err;


	switch (cmd->cmd) {
		case DEV_STOP:
			/*
			 * Stop the current search by 
			 *
			 */
			sstate->flags &= ~DEV_FLAG_RUNNING;
			break;
	
		case DEV_TERM:
			break;

		case DEV_START:
			/*
	 		 * Start the emulated device for now.
	 	  	 * XXX do this for real later.
	 		 */
			err = odisk_init(&sstate->ostate);
			if (err) {
				/* XXX log */
				/* XXX crap !! */
				return;
			}
			sstate->ver_no = cmd->id;
			sstate->flags |= DEV_FLAG_RUNNING;
			break;

		case DEV_SEARCHLET:
			sstate->ver_no = cmd->id;


			break;

		default:
			printf("unknown command %d \n", cmd->cmd);
			break;

	}

}

/*
 * This is the main thread that executes a "search" on a device.
 * This interates it handles incoming messages as well as processing
 * object.
 */

static void *
device_main(void *arg)
{
	search_state_t *sstate;
	dev_cmd_data_t *cmd;
	obj_data_t*	new_obj;
	int		err;
	int		any;
	pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;
	struct timeval now;
	struct timespec timeout;
	struct timezone tz;

	tz.tz_minuteswest = 0;
	tz.tz_dsttime = 0;

	sstate = (search_state_t *)arg;

	/*
	 * XXX need to open comm channel with device
	 */

	
	while (1) {
		any = 0;

		cmd = (dev_cmd_data_t *)ring_deq(sstate->control_ops);
		if (cmd != NULL) {
			any = 1;
			dev_process_cmd(sstate, cmd);
			free(cmd);
		}

			
		/*
		 * XXX look for data from device to process.
		 */
		if (sstate->flags & DEV_FLAG_RUNNING) {
			err = odisk_next_obj(&new_obj, sstate->ostate);
			printf("main: obbj \n");
			if (err == ENOENT) {
				/*
				 * We have processed all the objects,
				 * clear the running and set the complete
				 * flags.
				 */
				sstate->flags &= ~DEV_FLAG_RUNNING;
				sstate->flags |= DEV_FLAG_COMPLETE;
				/* XXX send complete message */
			} else if (err) {
				printf("dmain: failed to get obj !! \n");
				/* sleep(1); */
				continue;
			} else {
				any = 1;
				/* XXX process the object */

				/* XXX add vnum !!! */
				err = sstub_send_obj(sstate->comm_cookie, 
						new_obj, sstate->ver_no);
				if (err) {
					/* XXX handle overflow gracefully !!! */
					/* XXX log */
	
				}
			}

		}

		/*
		 * If we didn't have any work to process this time around,
		 * then we sleep on a cond variable for a small amount
		 * of time.
		 */
		/* XXX move mutex's to the state data structure */
		if (!any) {
			gettimeofday(&now, &tz);
			pthread_mutex_lock(&mut);
			timeout.tv_sec = now.tv_sec + 1;
			timeout.tv_nsec = now.tv_usec * 1000;
	
			pthread_cond_timedwait(&cond, &mut, &timeout);
			pthread_mutex_unlock(&mut);
		}
	}
}





/*
 * This is the callback that is called when a new connection
 * has been established at the network layer.  This creates
 * new search contect and creates a thread to process
 * the data. 
 */


int
search_new_conn(void *comm_cookie, void **app_cookie)
{
	search_state_t *	sstate;
	int			err;

	sstate = (search_state_t *) malloc(sizeof(*sstate));	
	if (sstate == NULL) {
		*app_cookie = NULL;
		return (ENOMEM);
	}

	/*
	 * init the ring to hold the queue of pending operations.
	 */
	err = ring_init(&sstate->control_ops);
	if (err) {
		free(sstate);
		*app_cookie = NULL;
		return (ENOENT);
	}

	sstate->flags = 0;
	sstate->comm_cookie = comm_cookie;

	/*
	 * Create a new thread that handles the searches for this current
	 * search.  (We probably want to make this a seperate process ??).
	 */

	err = pthread_create(&sstate->thread_id, NULL, device_main, 
			    (void *)sstate);
	if (err) {
		/* XXX log */
		free(sstate);
		*app_cookie = NULL;
		return (ENOENT);
	}

	/* set the  cookie and return */
	*app_cookie = sstate;
	return(0);
}




int
search_get_stats(void *app_cookie, int gen_num)
{
	int len;
	search_state_t *	sstate;
	dev_stats_t	*stats;
	int		err;

	sstate = (search_state_t *)app_cookie;
	printf("get state \n");

	len = sizeof(*stats);
	stats = (dev_stats_t *)malloc(len);

	stats->ds_num_filters = 0;

	err = sstub_send_stats(sstate->comm_cookie, stats, len);

	return(0);
}


/*
 * a request to get the characteristics of the device.
 */
int
search_get_char(void *app_cookie, int gen_num)
{
	device_char_t		dev_char;
	search_state_t *	sstate;
	u_int64_t 		val;
	int			err;


	printf("get char \n");

	sstate = (search_state_t *)app_cookie;

	dev_char.dc_isa = DEV_ISA_IA32;
	dev_char.dc_speed = (r_cpu_freq(&val) ? 0 : val);
	dev_char.dc_mem  =  (r_freemem(&val) ? 0 : val);

	/* XXX */
	err = sstub_send_dev_char(sstate->comm_cookie, &dev_char);

	return 0;
}

int
search_close_conn(void *app_cookie)
{
	printf("close_conn states \n");
}


/*
 * This releases an object that is no longer needed.
 */

int
search_release_obj(void *app_cookie, obj_data_t *obj)
{

	if (obj->data != NULL) {
		free(obj->data);
	}
	if (obj->attr_info.attr_data != NULL) {
		free(obj->attr_info.attr_data);
	}
	free(obj);
	return(0);

}


int
search_set_list(void *app_cookie, int gen_num)
{
	printf("XXX set list \n");
}




#ifdef	XXX_LATER

int
device_statistics(device_state_t *dev,
		  dev_stats_t *dev_stats, int *stat_len)
{
	filter_info_t *cur_filter;
	filter_stats_t *cur_filter_stats;
	rtime_t total_obj_time = 0;

	/* check args */
	if(!dev) return EINVAL;
	if(!dev_stats) return EINVAL;
	if(!stat_len) return EINVAL;
	if(*stat_len < sizeof(dev_stats_t)) return ENOSPC;

	memset(dev_stats, 0, *stat_len);
	
	cur_filter = dev->sc->bg_froot;
	cur_filter_stats = dev_stats->ds_filter_stats;
	while(cur_filter != NULL) {
		/* aggregate device stats */
		dev_stats->ds_num_filters++;
		/* make sure we have room for this filter */
		if(*stat_len < DEV_STATS_SIZE(dev_stats->ds_num_filters)) {
			return ENOSPC;
		}
		dev_stats->ds_objs_processed += cur_filter->fi_called;
		dev_stats->ds_objs_dropped += cur_filter->fi_drop;
		dev_stats->ds_system_load = 1; /* XXX FIX-RW */
		total_obj_time += cur_filter->fi_time_ns;
		
		/* fill in this filter stats */
		strncpy(cur_filter_stats->fs_name, cur_filter->fi_name, MAX_FILTER_NAME);
		cur_filter_stats->fs_name[MAX_FILTER_NAME-1] = '\0';
		cur_filter_stats->fs_objs_processed = cur_filter->fi_called;
		cur_filter_stats->fs_objs_dropped = cur_filter->fi_drop;
		cur_filter_stats->fs_avg_exec_time =  
			cur_filter->fi_time_ns / cur_filter->fi_called;

		cur_filter_stats++;		
		cur_filter = cur_filter->fi_next;
	}

	dev_stats->ds_avg_obj_time = total_obj_time / dev_stats->ds_objs_processed;
	/* set number of bytes used */
	*stat_len = DEV_STATS_SIZE(dev_stats->ds_num_filters);


	return 0;
}
#endif
