#ifndef _LIB_SEARCH_PRIV_H_
#define _LIB_SEARCH_PRIV_H_


/*
 * This is header file defines the internal state that is used
 * for the searchlet library.
 */


/*
 * This structure keeps track of the state associated with each
 * of the storage devices.
 */
#define	DEV_FLAG_RUNNING		0x01	
#define	DEV_FLAG_COMPLETE		0x02	
struct search_context;

typedef struct device_handle {
	struct device_handle * 	next;
	unsigned int		flags;
	void *			dev_handle;
	int			ver_no;
	struct search_context *	sc;
} device_handle_t;


typedef enum {
	SS_ACTIVE,		/* a search is currently in progress */
	SS_DONE,		/* search active, all object are processed */
	SS_EMPTY,
	SS_SHUTDOWN,	
	SS_IDLE	
} search_status_t;


typedef struct {
	obj_data_t *		obj;
	int			ver_num;
} obj_info_t;
/*
 * This defines the structures that keeps track of the current search
 * context.  This is the internal state that is kept for consistency,
 * etc.
 */

#define	OBJ_QUEUE_SIZE		1024
typedef struct search_context {
	int			cur_search_id;	/* ID of current search */
	device_handle_t *	dev_list;
	search_status_t		cur_status;	/* current status of search */
	ring_data_t *		proc_ring;	/* processed objects */
	ring_data_t *		unproc_ring;	/* unprocessed objects */
	ring_data_t *		bg_ops;	/* unprocessed objects */
	unsigned long		bg_status;
	void *		bg_froot; /* filter_info_t -RW */
} search_context_t;

/*
 * These are the prototypes of the device operations that
 * in the file ls_device.c
 */
extern int dev_new_obj_cb(void *hcookie, obj_data_t *odata, int vno);

/*
 * These are background processing functions.
 */
extern int bg_init(search_context_t *sc, int id);
extern int bg_set_searchlet(search_context_t *sc, int id, char *filter_name,
			char *spec_name);


#endif
