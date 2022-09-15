#ifndef NGX_HTTP_TEST_STATUS_SHM_H
#define NGX_HTTP_TEST_STATUS_SHM_H

typedef struct {
    ngx_str_t   *name;
    ngx_uint_t   max_size;
    ngx_uint_t   used_size;
    ngx_uint_t   used_node;

    ngx_uint_t   filter_used_size;
    ngx_uint_t   filter_used_node;
} ngx_http_test_traffic_status_shm_info_t;


ngx_int_t ngx_http_test_traffic_status_shm_add_server(ngx_http_request_t *r);
ngx_int_t ngx_http_test_traffic_status_shm_add_filter(ngx_http_request_t *r);
ngx_int_t ngx_http_test_traffic_status_shm_add_upstream(ngx_http_request_t *r);


void ngx_http_test_traffic_status_shm_info_node(ngx_http_request_t *r,
    ngx_http_test_traffic_status_shm_info_t *shm_info, ngx_rbtree_node_t *node);
void ngx_http_test_traffic_status_shm_info(ngx_http_request_t *r,
    ngx_http_test_traffic_status_shm_info_t *shm_info);

#endif