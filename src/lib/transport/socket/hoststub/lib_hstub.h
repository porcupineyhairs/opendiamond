#ifndef	_LIB_HSTUB_H_
#define	_LIB_HSTUB_H_


typedef	int (*hstub_new_obj_fn)(void *hcookie, obj_data_t *odata, int vno);
typedef	void (*hstub_log_data_fn)(void *hcookie, char *data, int len, int dev);
typedef	void (*hstub_search_done_fn)(void *hcookie, int ver_num);

typedef	void (*hstub_rleaf_done_fn)(void *hcookie, int err, 
                dctl_data_type_t dtype, int len, char *data, int32_t opid);
typedef	void (*hstub_wleaf_done_fn)(void *hcookie, int err, int32_t opid);
typedef	void (*hstub_lnodes_done_fn)(void *hcookie, int err, int num_ents,
                dctl_entry_t *data, int32_t opid);
typedef	void (*hstub_lleafs_done_fn)(void *hcookie, int err, int num_ents,
                dctl_entry_t *data, int32_t opid);


typedef struct {
	hstub_new_obj_fn		    new_obj_cb;
	hstub_log_data_fn		    log_data_cb;
	hstub_search_done_fn	    search_done_cb;
	hstub_rleaf_done_fn		    rleaf_done_cb;
	hstub_wleaf_done_fn		    wleaf_done_cb;
	hstub_lnodes_done_fn		lnode_done_cb;
	hstub_lleafs_done_fn		lleaf_done_cb;
} hstub_cb_args_t;


/*
 * This structure keeps track of the state associated with each
 * of the storage devices.
 */

void *	device_init(int id, uint32_t devid, void *hcookie, 
			hstub_cb_args_t *cbs);
int device_stop(void *dev, int id);
int device_terminate(void *dev, int id);
int device_start(void *dev, int id);
int device_set_searchlet(void *dev, int id, char *filter, char *spec);
int device_characteristics(void *handle, device_char_t *dev_chars);
int device_statistics(void *dev, dev_stats_t *dev_stats, 
		int *stat_len);
int device_set_log(void *handle, uint32_t level, uint32_t src);

int device_write_leaf(void *dev, char *path, int len, char *data,
                int32_t opid);
int device_read_leaf(void *dev, char *path, int32_t opid);
int device_list_nodes(void *dev, char *path, int32_t opid);
int device_list_leafs(void *dev, char *path, int32_t opid);
int device_new_gid(void *handle, int id, groupid_t gid);
int device_clear_gids(void *handle, int id);

#endif	/* _LIB_HSTUB_H_ */



