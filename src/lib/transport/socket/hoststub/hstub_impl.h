/*
 *  The OpenDiamond Platform for Interactive Search
 *  Version 3
 *
 *  Copyright (c) 2002-2007 Intel Corporation
 *  Copyright (c) 2006 Larry Huston <larry@thehustons.net>
 *  Copyright (c) 2007-2008 Carnegie Mellon University
 *  All rights reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef	_LIB_HSTUB_IMPL_H_
#define	_LIB_HSTUB_IMPL_H_

#include <minirpc/minirpc.h>
#include "ring.h"

/*
 * This structure keeps track of the state associated with each
 * of the storage devices.
 */

typedef enum {
    DATA_RX_NO_PENDING,
    DATA_RX_HEADER,
    DATA_RX_ATTR,
    DATA_RX_DATA,
} data_rx_state_t;

/* flag definitons */
#define	CINFO_BLOCK_OBJ		0x02
#define	CINFO_PENDING_CREDIT	0x04
#define	CINFO_DOWN		0x08

typedef struct conn_info {
	pthread_mutex_t		mutex; /* protects 'flags' */
	int	 		flags;
	uint32_t		ipv4addr; /* used by device_characteristics() */
        sig_val_t		session_nonce; /* for pairing control and data conns */
	struct mrpc_connection *rpc_client;
	int			data_fd;
	data_rx_state_t		data_rx_state;
	obj_header_t		data_rx_header;
	int			data_rx_offset;
	obj_data_t *		data_rx_obj;
	credit_count_msg_t 	cc_msg;
	int			cc_counter;
	int			obj_limit;
	uint32_t            	stat_log_rx;
	uint64_t            	stat_log_byte_rx;
	uint32_t            	stat_control_rx;
	uint64_t            	stat_control_byte_rx;
	uint32_t            	stat_control_tx;
	uint64_t            	stat_control_byte_tx;
	uint32_t            	stat_obj_rx;
	uint64_t            	stat_obj_total_byte_rx;
	uint64_t            	stat_obj_hdr_byte_rx;
	uint64_t            	stat_obj_attr_byte_rx;
	uint64_t            	stat_obj_data_byte_rx;
} conn_info_t;


typedef struct sdevice_state {
	struct sdevice_state * 	next;
	pthread_t	        thread_id;
	ring_data_t *	    	obj_ring;
	conn_info_t 	    	con_data;
	void *		    	hcookie;
	device_char_t		dev_char;	/* cached device chars */
	int		       	stat_size;	/* size of caches stats */
	dev_stats_t *		dstats;		/* caches stats */
	void *			dctl_cookie;
	void *			log_cookie;
	hstub_cb_args_t		cb;
} sdevice_state_t;

/*
 * Functions availabe in hstub_api.c
 */
int rpc_postproc(const char *func, mrpc_status_t ret);

/*
 * Functions availabe in hstub_main.c
 */
void *hstub_main(void *arg);

/*
 * Functions available in hstub_data.c
 */
int hstub_read_data(sdevice_state_t *dev);
int hstub_write_data(sdevice_state_t *dev);

/*
 * Functions available in hstub_socket.h.
 */
int hstub_establish_connection(conn_info_t *cinfo, const char *host);

#endif	/* !_LIB_HSTUB_IMPL_H_ */



