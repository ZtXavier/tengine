#ifndef  _NGX_HTTP_TEST_STATUS_MODULE_H_
#define _NGX_HTTP_TEST_STATUS_MODULE_H_


#include<ngx_core.h>
#include<ngx_config.h>
#include<ngx_http.h>
#include<nginx.h>


#include"ngx_http_test_status_module_node.h"

#define NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_NO          0
#define NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_UA          1
#define NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_UG          2
#define NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_CC          3
#define NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_FG          4

#define NGX_HTTP_TEST_TRAFFIC_STATUS_AVERAGE_METHOD_AMM   0
#define NGX_HTTP_TEST_TRAFFIC_STATUS_AVERAGE_METHOD_WMA   1

#define NGX_HTTP_TEST_STATUS_FORMAT_NONE    0
#define NGX_HTTP_TEST_STATUS_FORMAT_HTML    1
#define NGX_HTTP_TEST_STATUS_FORMAT_PROMETHEUS      2


#define NGX_HTTP_TEST_TRAFFIC_STATUS_DEFAULT_SHM_NAME "ngx_test_traffic_status"
#define NGX_HTTP_TEST_TRAFFIC_STATUS_DEFAULT_SHM_SIZE     0xfffff


#define NGX_HTTP_TEST_TRAFFIC_STATUS_KEY_SEPARATOR        (u_char) 0x1f

#define NGX_HTTP_TEST_TRAFFIC_STATUS_NODE_NONE            0
#define NGX_HTTP_TEST_TRAFFIC_STATUS_NODE_FIND            1

#define NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAMS            (u_char *) "NO\0UA\0UG\0CC\0FG\0"

#define ngx_http_test_traffic_status_group_to_string(n) (u_char *) (          \
    (n > 4)                                                                    \
    ? NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAMS                                  \
    : NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAMS + 3 * n                          \
)

#define ngx_http_test_traffic_status_string_to_group(s) (unsigned) (          \
{                                                                              \
    unsigned n = NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_NO;                    \
    if (*s == 'N' && *(s + 1) == 'O') {                                        \
        n = NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_NO;                         \
    } else if (*s == 'U' && *(s + 1) == 'A') {                                 \
        n = NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_UA;                         \
    } else if (*s == 'U' && *(s + 1) == 'G') {                                 \
        n = NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_UG;                         \
    } else if (*s == 'C' && *(s + 1) == 'C') {                                 \
        n = NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_CC;                         \
    } else if (*s == 'F' && *(s + 1) == 'G') {                                 \
        n = NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_FG;                         \
    }                                                                          \
    n;                                                                         \
}                                                                              \
)

#define ngx_http_test_traffic_status_triangle(n) (unsigned) (                 \
    n * (n + 1) / 2                                                            \
)



#define ngx_http_test_traffic_status_add_rc(s, n) {                           \
    if(s < 200) {n->stat_1xx_counter++;}                                       \
    else if(s < 300) {n->stat_2xx_counter++;}                                  \
    else if(s < 400) {n->stat_3xx_counter++;}                                  \
    else if(s < 500) {n->stat_4xx_counter++;}                                  \
    else {n->stat_5xx_counter++;}                                              \
}


#define ngx_http_test_traffic_status_add_oc(o, c) {                 \
    if (o->stat_in_bytes > c->stat_in_bytes) {                                 \
        c->stat_in_bytes_oc++;                                                 \
    }                                                                          \
    if (o->stat_out_bytes > c->stat_out_bytes) {                               \
        c->stat_out_bytes_oc++;                                                \
    }                                                                          \
    if (o->stat_1xx_counter > c->stat_1xx_counter) {                           \
        c->stat_1xx_counter_oc++;                                              \
    }                                                                          \
    if (o->stat_2xx_counter > c->stat_2xx_counter) {                           \
        c->stat_2xx_counter_oc++;                                              \
    }                                                                          \
    if (o->stat_3xx_counter > c->stat_3xx_counter) {                           \
        c->stat_3xx_counter_oc++;                                              \
    }                                                                          \
    if (o->stat_4xx_counter > c->stat_4xx_counter) {                           \
        c->stat_4xx_counter_oc++;                                              \
    }                                                                          \
    if (o->stat_5xx_counter > c->stat_5xx_counter) {                           \
        c->stat_5xx_counter_oc++;                                              \
    }                                                                              \
}


typedef struct {
     ngx_rbtree_t                           *rbtree;

     /* array of ngx_http_test_traffic_status_filter_t */
    ngx_array_t                            *filter_keys;

    /* array of ngx_http_test_traffic_status_limit_t */
    ngx_array_t                            *limit_traffics;

    /* array of ngx_http_test_traffic_status_limit_t */
    ngx_array_t                            *limit_filter_traffics;

    /* array of ngx_http_test_traffic_status_filter_match_t */
    ngx_array_t                            *filter_max_node_matches; 

    ngx_uint_t                              filter_max_node;

    ngx_flag_t                                  enable;
    ngx_flag_t                                  filter_check_duplicate;
    ngx_flag_t                                  limit_check_duplicate;
    ngx_shm_zone_t                         *shm_zone;
    ngx_str_t                                     shm_name;
    ssize_t                                         shm_size;

} ngx_http_test_traffic_status_ctx_t;

typedef struct {
    ngx_shm_zone_t                     *shm_zone;
    ngx_str_t                               shm_name;
    ngx_flag_t                              enable;
    ngx_flag_t                              filter;
    ngx_flag_t                              filter_host;
    ngx_flag_t                              filter_check_duplicate;

    /* array of ngx_http_test_traffic_status_filter_t */
    ngx_array_t                            *filter_keys;

    /* array of ngx_http_test_traffic_status_filter_variable_t */
    ngx_array_t                            *filter_vars;

    ngx_flag_t                              limit;
    ngx_flag_t                              limit_check_duplicate;

    /* array of ngx_http_test_traffic_status_limit_t */
    ngx_array_t                            *limit_traffics;

    /* array of ngx_http_test_traffic_status_limit_t */
    ngx_array_t                            *limit_filter_traffics;

    ngx_http_test_traffic_status_node_t    stats;
    ngx_msec_t                              start_msec;
    ngx_flag_t                              format;
    ngx_str_t                               jsonp;
    ngx_str_t                               sum_key;

    ngx_flag_t                              average_method;
    ngx_msec_t                              average_period;

    /* array of ngx_http_test_traffic_status_node_histogram_t */
    ngx_array_t                            *histogram_buckets;

    ngx_flag_t                              bypass_limit;
    ngx_flag_t                              bypass_stats;

    ngx_rbtree_node_t                     **node_caches;
} ngx_http_test_traffic_status_loc_conf_t;


extern ngx_module_t ngx_http_test_traffic_status_module;

ngx_msec_t ngx_http_test_traffic_status_current_msec(void);
ngx_msec_int_t ngx_http_test_traffic_status_request_time(ngx_http_request_t *r);
ngx_msec_int_t ngx_http_test_traffic_status_upstream_response_time(ngx_http_request_t *r);


#endif