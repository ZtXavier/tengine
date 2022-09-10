#ifndef  _NGX_HTTP_TEST_STATUS_MODULE_H_
#define _NGX_HTTP_TEST_STATUS_MODULE_H_


#include<ngx_config.h>
#include<ngx_core.h>
#include<ngx_http.h>
#include<nginx.h>


#define NGX_HTTP_TEST_STATUS_FORMAT_NONE    0
#define NGX_HTTP_TEST_STATUS_FORMAT_HTML    1
#define NGX_HTTP_TEST_STATUS_FORMAT_PROMETHEUS      2


#define NGX_HTTP_TEST_TRAFFIC_STATUS_DEFAULT_SHM_NAME "ngx_test_traffic_status"
#define NGX_HTTP_TEST_TRAFFIC_STATUS_DEFAULT_SHM_SIZE     0xfffff

#define NGX_HTTP_TEST_TRAFFIC_STATUS_DEFAULT_QUEUE_LEN    64
#define NGX_HTTP_TEST_TRAFFIC_STATUS_DEFAULT_BUCKET_LEN   32



#define ngx_http_test_traffic_status_add_rc(s, n) {                           \
    if(s < 200) {n->stat_1xx_counter++;}                                       \
    else if(s < 300) {n->stat_2xx_counter++;}                                  \
    else if(s < 400) {n->stat_3xx_counter++;}                                  \
    else if(s < 500) {n->stat_4xx_counter++;}                                  \
    else {n->stat_5xx_counter++;}                                              \
}


#define ngx_http_test_traffic_status_add_oc(o, c) {                           \
    if (o->stat_request_counter > c->stat_request_counter) {                   \
        c->stat_request_counter_oc++;                                          \
    }                                                                          \
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
    }                                                                          \
    if (o->stat_request_time_counter > c->stat_request_time_counter) {         \
        c->stat_request_time_counter_oc++;                                     \
    }                                                                          \
}

typedef struct {
    ngx_msec_t                                             time;
    ngx_msec_int_t                                         msec;
} ngx_http_test_traffic_status_node_time_t;


typedef struct {
    ngx_http_test_traffic_status_node_time_t              times[NGX_HTTP_TEST_TRAFFIC_STATUS_DEFAULT_QUEUE_LEN];
    ngx_int_t                                              front;
    ngx_int_t                                              rear;
    ngx_int_t                                              len;
} ngx_http_test_traffic_status_node_time_queue_t;


typedef struct {
    ngx_msec_int_t                                         msec;
    ngx_atomic_t                                           counter;
} ngx_http_test_traffic_status_node_histogram_t;


typedef struct {
    ngx_http_test_traffic_status_node_histogram_t         buckets[NGX_HTTP_TEST_TRAFFIC_STATUS_DEFAULT_BUCKET_LEN];
    ngx_int_t                                              len;
} ngx_http_test_traffic_status_node_histogram_bucket_t;


typedef struct {
    /* unsigned type:5 */
    unsigned                                               type;
    ngx_atomic_t                                           response_time_counter;
    ngx_msec_t                                             response_time;
    ngx_http_test_traffic_status_node_time_queue_t        response_times;
    ngx_http_test_traffic_status_node_histogram_bucket_t  response_buckets;
} ngx_http_test_traffic_status_node_upstream_t;




typedef struct {
    u_char                                                     color;
    ngx_atomic_t                                           stat_request_counter;
    ngx_atomic_t                                           stat_in_bytes;
    ngx_atomic_t                                           stat_out_bytes;
    ngx_atomic_t                                           stat_1xx_counter;
    ngx_atomic_t                                           stat_2xx_counter;
    ngx_atomic_t                                           stat_3xx_counter;
    ngx_atomic_t                                           stat_4xx_counter;
    ngx_atomic_t                                           stat_5xx_counter;

    ngx_atomic_t                                           stat_request_time_counter;
    ngx_msec_t                                             stat_request_time;
    ngx_http_test_traffic_status_node_time_queue_t        stat_request_times;
    ngx_http_test_traffic_status_node_histogram_bucket_t  stat_request_buckets;

    /* deals with the overflow of variables */
    ngx_atomic_t                                           stat_request_counter_oc;
    ngx_atomic_t                                           stat_in_bytes_oc;
    ngx_atomic_t                                           stat_out_bytes_oc;
    ngx_atomic_t                                           stat_1xx_counter_oc;
    ngx_atomic_t                                           stat_2xx_counter_oc;
    ngx_atomic_t                                           stat_3xx_counter_oc;
    ngx_atomic_t                                           stat_4xx_counter_oc;
    ngx_atomic_t                                           stat_5xx_counter_oc;
    ngx_atomic_t                                           stat_request_time_counter_oc;
    ngx_atomic_t                                           stat_response_time_counter_oc;

    ngx_http_test_traffic_status_node_upstream_t          stat_upstream;
    u_short                                                len;
    u_char                                                 data[1];
} ngx_http_test_traffic_status_node_t;


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


