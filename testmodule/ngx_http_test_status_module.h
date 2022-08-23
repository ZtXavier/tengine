#include<ngx_config.h>
#include<ngx_core.h>
#include<ngx_http.h>

typedef struct {
    ngx_uint_t      requests;
    ngx_uint_t      bandwdith;
    ngx_uint_t      max_bandwidth;
    ngx_uint_t      max_active;
    ngx_uint_t      traffic;
}ngx_http_new_status_data_t;


typedef struct {
    ngx_rbtree_node_t          node;
    ngx_queue_t                   queue;
    ngx_uint_t                      count; 
    ngx_http_new_status_data_t      data;
    ngx_uint_t                      active;
    ngx_uint_t                      last_traffic;
    ngx_msec_t                      last_traffic_start;
    ngx_msec_t                      last_traffic_update;
    ngx_uint_t                      len;
    u_char                          key[0];
}ngx_http_new_status_node_t;

typedef struct {
    ngx_rbtree_t            rbtree;
    ngx_rbtree_node_t       sentinel;
    ngx_queue_t                 queue;
    time_t                          expire_lock;
}ngx_http_new_status_sh_t;

typedef struct {
    ngx_http_new_status_sh_t        *sh;
    ngx_slab_pool_t                      *shpool;
    ngx_shm_zone_t                      *shm_zone;
    ngx_http_complex_value_t         key;

}ngx_http_new_status_zone_t;

typedef struct {
    ngx_array_t                     zones;
    ngx_msec_t                      interval;
    time_t                          lock_time;
} ngx_http_new_status_main_conf_t;

typedef struct {
    ngx_array_t                     req_zones;
} ngx_http_new_status_ctx_t;

typedef struct {
    ngx_array_t                     zones;

    ngx_msec_t                      interval;
    time_t                          lock_time;
} ngx_http__status_main_conf_t;


typedef struct ngx_http_new_status_loc_conf_s {
    ngx_array_t                     req_zones;
    ngx_http_new_status_loc_conf_t *parent;
}ngx_http_new_status_loc_conf_t;

static ngx_http_module_t ngx_http_new_status_module_ctx = {
    NULL,
    ngx_http_new_status_init,
    ngx_http_new_status_create_main_conf,
    ngx_http_new_status_init_main_conf,
    NULL,
    NULL,
    ngx_http_new_status_create_loc_conf,
    ngx_http_new_status_merge_loc_conf
};




static ngx_command_t ngx_http_new_status_commands[] = {
    
};


ngx_module_t ngx_http_new_status_module = {
    NGX_MODULE_V1,
    &ngx_http_new_status_module_ctx,
    ngx_http_new_status_commands,
    NGX_HTTP_MODULE,                          /* module type */
    NULL,                                     /* init master */
    NULL,                                     /* init module */
    NULL,                                     /* init process */
    NULL,                                     /* init thread */
    NULL,                                     /* exit thread */
    NULL,                                     /* exit process */
    NULL,                                     /* exit master */
    NGX_MODULE_V1_PADDING
};