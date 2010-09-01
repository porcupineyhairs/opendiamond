/*
 *  The OpenDiamond Platform for Interactive Search
 *  Version 4
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  Copyright (c) 2006 Larry Huston <larry@thehustons.net>
 *  Copyright (c) 2007-2009 Carnegie Mellon University
 *  All rights reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef _SEARCH_STATE_H_
#define _SEARCH_STATE_H_

#include <glib.h>
#include <stdbool.h>

/*
 * some of the default constants for packet processing 
 */
#define	SSTATE_DEFAULT_PEND_MAX	30


/* do we try to continue evaluating objects when stalled */ 
#define	SSTATE_DEFAULT_WORKAHEAD	0

/* name to use for logs */
#define LOG_PREFIX 	"adiskd"

enum split_types_t {
	SPLIT_TYPE_FIXED = 0,	/* Defined fixed ratio of work */
	SPLIT_TYPE_DYNAMIC	/* use dynamic optimization */
};


#define	SPLIT_DEFAULT_BP_THRESH	15
#define	SPLIT_DEFAULT_TYPE		(SPLIT_TYPE_FIXED)
#define	SPLIT_DEFAULT_RATIO		(100)
#define	SPLIT_DEFAULT_AUTO_STEP		5
#define	SPLIT_DEFAULT_PEND_LOW		200
#define	SPLIT_DEFAULT_MULT		20
#define	SPLIT_DEFAULT_PEND_HIGH		10

#define DEV_FLAG_RUNNING                0x01
#define DEV_FLAG_COMPLETE               0x02



typedef struct search_state {
	void           *comm_cookie;	/* cookie from the communication lib */
	pthread_t       thread_id;
	unsigned int    flags;
	struct odisk_state *ostate;
	struct ceval_state *cstate;
	session_info_t		cinfo;			/* used for session id */
	GAsyncQueue    *control_ops;
	pthread_mutex_t log_mutex;
	pthread_cond_t  log_cond;
	filter_data_t  *fdata;
	uint            obj_processed;		/* really objects read */
	uint            obj_dropped;
	uint            obj_passed;
	uint            obj_skipped;
	uint            network_stalls;
	uint            tx_full_stalls;
	uint            tx_idles;
	uint            pend_objs;
	float           pend_compute;
	uint            pend_max;
	uint            work_ahead;	/* do we work ahead for caching */
	uint            split_type;	/* policy for the splitting */
	uint            split_ratio;	/* amount of computation to do local */
	uint            split_mult;	/* multiplier for queue size */
	uint            split_auto_step;	/* step to increment ration
						 * by */
	uint            split_bp_thresh;	/* below, not enough work for 
						 * host */
	uint            avg_int_ratio;	/* average ratio for this run */
	uint            smoothed_int_ratio;	/* integer smoothed ratio */
	float           smoothed_ratio;	/* smoothed value */
	uint            old_proc;	/* last number run */
	float           avg_ratio;	/* floating point avg ratio */
	unsigned char  *sig;
	user_state_t	user_state;
	session_variables_state_t *session_variables_state;
} search_state_t;

#endif				/* ifndef _SEARCH_STATE_H_ */