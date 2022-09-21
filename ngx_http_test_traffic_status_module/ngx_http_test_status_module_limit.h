#ifndef NGX_HTTP_TEST_LIMIT_H
#define NGX_HTTP_TEST_LIMIT_H


typedef struct {
    ngx_http_complex_value_t     key;
    ngx_http_complex_value_t     variable;
    ngx_atomic_t                 size;
    ngx_uint_t                   code;
    unsigned                     type;        /* unsigned type:5 */
} ngx_http_test_traffic_status_limit_t;


ngx_int_t ngx_http_test_traffic_status_limit_handler(ngx_http_request_t *r);
ngx_int_t ngx_http_test_traffic_status_limit_handler_traffic(ngx_http_request_t *r,
    ngx_array_t *traffics);

ngx_int_t ngx_http_test_traffic_status_limit_traffic_unique(
    ngx_pool_t *pool, ngx_array_t **keys);
char *ngx_http_test_traffic_status_limit_traffic(ngx_conf_t *cf,
    ngx_command_t *cmd, void *conf);
char *ngx_http_test_traffic_status_limit_traffic_by_set_key(ngx_conf_t *cf,
    ngx_command_t *cmd, void *conf);


#endif 