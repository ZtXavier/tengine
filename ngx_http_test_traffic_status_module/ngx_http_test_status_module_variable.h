#ifndef NGX_HTTP_TEST_STATUS_MODULE_VARIABLE_H
#define NGX_HTTP_TEST_STATUS_MODULE_VARIABLE_H



ngx_int_t ngx_http_test_traffic_status_node_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);


ngx_int_t ngx_http_test_traffic_status_add_variables(ngx_conf_t *cf);


#endif