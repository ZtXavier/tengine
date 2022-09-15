#include"ngx_http_test_status_module.h"
#include"ngx_http_test_status_module_node.h"
#include"ngx_http_test_status_module_prome.h"
#include"ngx_http_test_status_module_shm.h"



u_char *
ngx_http_test_traffic_status_display_prometheus_set_main(ngx_http_request_t *r,
    u_char *buf)
{
    ngx_atomic_int_t                           ap, hn, ac, rq, rd, wr, wa;
    ngx_http_test_traffic_status_loc_conf_t  *vtscf;
    ngx_http_test_traffic_status_shm_info_t  *shm_info;

    vtscf = ngx_http_get_module_loc_conf(r, ngx_http_test_traffic_status_module);

    ap = *ngx_stat_accepted;
    hn = *ngx_stat_handled;
    ac = *ngx_stat_active;
    rq = *ngx_stat_requests;
    rd = *ngx_stat_reading;
    wr = *ngx_stat_writing;
    wa = *ngx_stat_waiting;

    shm_info = ngx_pcalloc(r->pool, sizeof(ngx_http_test_traffic_status_shm_info_t));
    if (shm_info == NULL) {
        return buf;
    }

    ngx_http_test_traffic_status_shm_info(r, shm_info);

    buf = ngx_sprintf(buf, NGX_HTTP_TEST_TRAFFIC_STATUS_PROMETHEUS_FMT_MAIN, &ngx_cycle->hostname,
                       "NGX_HTTP_TEST_MODULE_VERSION",NGINX_VERSION,
                      (double) vtscf->start_msec / 1000,
                      ap, ac, hn, rd, rq, wa, wr,
                      shm_info->name, shm_info->max_size,
                      shm_info->used_size, shm_info->used_node);

    return buf;
}


u_char *
ngx_http_test_traffic_status_display_prometheus_set_server_node(
    ngx_http_request_t *r,
    u_char *buf, ngx_str_t *key,
    ngx_http_test_traffic_status_node_t *vtsn)
{
    ngx_str_t                                               server;
/*     ngx_uint_t                                              i, n; */
    ngx_http_test_traffic_status_loc_conf_t               *vtscf; 
   /*  ngx_http_test_traffic_status_node_histogram_bucket_t  *b; */

    vtscf = ngx_http_get_module_loc_conf(r, ngx_http_test_traffic_status_module);

    server = *key;

    (void) ngx_http_test_traffic_status_node_position_key(&server, 1);

    buf = ngx_sprintf(buf, NGX_HTTP_TEST_TRAFFIC_STATUS_PROMETHEUS_FMT_SERVER,
                      &server, vtsn->stat_in_bytes,
                      &server, vtsn->stat_out_bytes,
                      &server, vtsn->stat_1xx_counter,
                      &server, vtsn->stat_2xx_counter,
                      &server, vtsn->stat_3xx_counter,
                      &server, vtsn->stat_4xx_counter,
                      &server, vtsn->stat_5xx_counter,
                      &server, (double) vtsn->stat_request_time_counter / 1000,
                      &server, (double) ngx_http_test_traffic_status_node_time_queue_average(
                                   &vtsn->stat_request_times, vtscf->average_method,
                                   vtscf->average_period) / 1000);



// #if (NGX_HTTP_CACHE)
//     buf = ngx_sprintf(buf, NGX_HTTP_test_TRAFFIC_STATUS_PROMETHEUS_FMT_SERVER_CACHE,
//                       &server, vtsn->stat_cache_miss_counter,
//                       &server, vtsn->stat_cache_bypass_counter,
//                       &server, vtsn->stat_cache_expired_counter,
//                       &server, vtsn->stat_cache_stale_counter,
//                       &server, vtsn->stat_cache_updating_counter,
//                       &server, vtsn->stat_cache_revalidated_counter,
//                       &server, vtsn->stat_cache_hit_counter,
//                       &server, vtsn->stat_cache_scarce_counter);
// #endif

    return buf;
}


u_char *
ngx_http_test_traffic_status_display_prometheus_set_server(ngx_http_request_t *r,
    u_char *buf, ngx_rbtree_node_t *node)
{
    ngx_str_t                                  key, escaped_key;
    ngx_http_test_traffic_status_ctx_t       *ctx;
    ngx_http_test_traffic_status_node_t      *vtsn;
    ngx_http_test_traffic_status_loc_conf_t  *vtscf;

    ctx = ngx_http_get_module_main_conf(r, ngx_http_test_traffic_status_module);

    vtscf = ngx_http_get_module_loc_conf(r, ngx_http_test_traffic_status_module);

    if (node != ctx->rbtree->sentinel) {
        vtsn = (ngx_http_test_traffic_status_node_t *) &node->color;

        if (vtsn->stat_upstream.type == NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_NO) {
            key.data = vtsn->data;
            key.len = vtsn->len;

            ngx_http_test_traffic_status_escape_prometheus(r->pool, &escaped_key, key.data, key.len);
            buf = ngx_http_test_traffic_status_display_prometheus_set_server_node(r, buf, &escaped_key, vtsn);

            /* calculates the sum */
            vtscf->stats.stat_request_counter += vtsn->stat_request_counter;
            vtscf->stats.stat_in_bytes += vtsn->stat_in_bytes;
            vtscf->stats.stat_out_bytes += vtsn->stat_out_bytes;
            vtscf->stats.stat_1xx_counter += vtsn->stat_1xx_counter;
            vtscf->stats.stat_2xx_counter += vtsn->stat_2xx_counter;
            vtscf->stats.stat_3xx_counter += vtsn->stat_3xx_counter;
            vtscf->stats.stat_4xx_counter += vtsn->stat_4xx_counter;
            vtscf->stats.stat_5xx_counter += vtsn->stat_5xx_counter;
            vtscf->stats.stat_request_time_counter += vtsn->stat_request_time_counter;
            ngx_http_test_traffic_status_node_time_queue_merge(
                &vtscf->stats.stat_request_times,
                &vtsn->stat_request_times, vtscf->average_period);

// #if (NGX_HTTP_CACHE)
//             vtscf->stats.stat_cache_miss_counter +=
//                                        vtsn->stat_cache_miss_counter;
//             vtscf->stats.stat_cache_bypass_counter +=
//                                        vtsn->stat_cache_bypass_counter;
//             vtscf->stats.stat_cache_expired_counter +=
//                                        vtsn->stat_cache_expired_counter;
//             vtscf->stats.stat_cache_stale_counter +=
//                                        vtsn->stat_cache_stale_counter;
//             vtscf->stats.stat_cache_updating_counter +=
//                                        vtsn->stat_cache_updating_counter;
//             vtscf->stats.stat_cache_revalidated_counter +=
//                                        vtsn->stat_cache_revalidated_counter;
//             vtscf->stats.stat_cache_hit_counter +=
//                                        vtsn->stat_cache_hit_counter;
//             vtscf->stats.stat_cache_scarce_counter +=
//                                        vtsn->stat_cache_scarce_counter;
// #endif
        }

        buf = ngx_http_test_traffic_status_display_prometheus_set_server(r, buf, node->left);
        buf = ngx_http_test_traffic_status_display_prometheus_set_server(r, buf, node->right);
    }

    return buf;
}



u_char *
ngx_http_test_traffic_status_display_prometheus_set_filter_node(
    ngx_http_request_t *r,
    u_char *buf, ngx_str_t *key,
    ngx_http_test_traffic_status_node_t *vtsn)
{
    ngx_str_t                                               filter, filter_name;
/*     ngx_uint_t                                              i, n; */
    ngx_http_test_traffic_status_loc_conf_t               *vtscf;
/*     ngx_http_test_traffic_status_node_histogram_bucket_t  *b; */

    vtscf = ngx_http_get_module_loc_conf(r, ngx_http_test_traffic_status_module);

    filter = filter_name = *key;

    (void) ngx_http_test_traffic_status_node_position_key(&filter, 1);
    (void) ngx_http_test_traffic_status_node_position_key(&filter_name, 2);

    buf = ngx_sprintf(buf, NGX_HTTP_TEST_TRAFFIC_STATUS_PROMETHEUS_FMT_FILTER,
                      &filter, &filter_name, vtsn->stat_in_bytes,
                      &filter, &filter_name, vtsn->stat_out_bytes,
                      &filter, &filter_name, vtsn->stat_1xx_counter,
                      &filter, &filter_name, vtsn->stat_2xx_counter,
                      &filter, &filter_name, vtsn->stat_3xx_counter,
                      &filter, &filter_name, vtsn->stat_4xx_counter,
                      &filter, &filter_name, vtsn->stat_5xx_counter,
                      &filter, &filter_name, (double) vtsn->stat_request_time_counter / 1000,
                      &filter, &filter_name,
                      (double) ngx_http_test_traffic_status_node_time_queue_average(
                          &vtsn->stat_request_times, vtscf->average_method,
                          vtscf->average_period) / 1000);

    // /* histogram */
    // b = &vtsn->stat_request_buckets;

    // n = b->len;

    // if (n > 0) {

    //     /* histogram:bucket */
    //     for (i = 0; i < n; i++) {
    //         buf = ngx_sprintf(buf,
    //                   NGX_HTTP_test_TRAFFIC_STATUS_PROMETHEUS_FMT_FILTER_HISTOGRAM_BUCKET,
    //                   &filter, &filter_name, (double) b->buckets[i].msec / 1000,
    //                   b->buckets[i].counter);
    //     }

    //     buf = ngx_sprintf(buf,
    //               NGX_HTTP_test_TRAFFIC_STATUS_PROMETHEUS_FMT_FILTER_HISTOGRAM_BUCKET_E,
    //               &filter, &filter_name, vtsn->stat_request_counter);

    //     /* histogram:sum */
    //     buf = ngx_sprintf(buf,
    //               NGX_HTTP_test_TRAFFIC_STATUS_PROMETHEUS_FMT_FILTER_HISTOGRAM_SUM,
    //               &filter, &filter_name, (double) vtsn->stat_request_time_counter / 1000);

    //     /* histogram:count */
    //     buf = ngx_sprintf(buf,
    //               NGX_HTTP_test_TRAFFIC_STATUS_PROMETHEUS_FMT_FILTER_HISTOGRAM_COUNT,
    //               &filter, &filter_name, vtsn->stat_request_counter);
    // }

// #if (NGX_HTTP_CACHE)
//     buf = ngx_sprintf(buf, NGX_HTTP_test_TRAFFIC_STATUS_PROMETHEUS_FMT_FILTER_CACHE,
//                       &filter, &filter_name, vtsn->stat_cache_miss_counter,
//                       &filter, &filter_name, vtsn->stat_cache_bypass_counter,
//                       &filter, &filter_name, vtsn->stat_cache_expired_counter,
//                       &filter, &filter_name, vtsn->stat_cache_stale_counter,
//                       &filter, &filter_name, vtsn->stat_cache_updating_counter,
//                       &filter, &filter_name, vtsn->stat_cache_revalidated_counter,
//                       &filter, &filter_name, vtsn->stat_cache_hit_counter,
//                       &filter, &filter_name, vtsn->stat_cache_scarce_counter);
// #endif

    return buf;
}


// u_char *
// ngx_http_test_traffic_status_display_prometheus_set_filter(ngx_http_request_t *r,
//     u_char *buf, ngx_rbtree_node_t *node)
// {
//     ngx_str_t                              key, escaped_key;
//     ngx_http_test_traffic_status_ctx_t   *ctx;
//     ngx_http_test_traffic_status_node_t  *vtsn;

//     ctx = ngx_http_get_module_main_conf(r, ngx_http_test_traffic_status_module);

//     if (node != ctx->rbtree->sentinel) {
//         vtsn = (ngx_http_test_traffic_status_node_t *) &node->color;

//         if (vtsn->stat_upstream.type == NGX_HTTP_test_TRAFFIC_STATUS_UPSTREAM_FG) {
//             key.data = vtsn->data;
//             key.len = vtsn->len;

//             ngx_http_test_traffic_status_escape_prometheus(r->pool, &escaped_key, key.data, key.len);
//             buf = ngx_http_test_traffic_status_display_prometheus_set_filter_node(r, buf, &escaped_key, vtsn);
//         }

//         buf = ngx_http_test_traffic_status_display_prometheus_set_filter(r, buf, node->left);
//         buf = ngx_http_test_traffic_status_display_prometheus_set_filter(r, buf, node->right);
//     }

//     return buf;
// }


// u_char *
// ngx_http_test_traffic_status_display_prometheus_set_upstream_node(
//     ngx_http_request_t *r,
//     u_char *buf, ngx_str_t *key,
//     ngx_http_test_traffic_status_node_t *vtsn)
// {
//     ngx_str_t                                               target, upstream, upstream_server;
//     ngx_uint_t                                              i, n, len;
//     ngx_atomic_t                                            time_counter;
//     ngx_http_test_traffic_status_loc_conf_t               *vtscf;
//     ngx_http_test_traffic_status_node_histogram_bucket_t  *b;

//     vtscf = ngx_http_get_module_loc_conf(r, ngx_http_test_traffic_status_module);

//     upstream = upstream_server = *key;

//     if (vtsn->stat_upstream.type == NGX_HTTP_test_TRAFFIC_STATUS_UPSTREAM_UG) {
//         (void) ngx_http_test_traffic_status_node_position_key(&upstream, 1);
//         (void) ngx_http_test_traffic_status_node_position_key(&upstream_server, 2);

//     } else if (vtsn->stat_upstream.type == NGX_HTTP_test_TRAFFIC_STATUS_UPSTREAM_UA) {
//         ngx_str_set(&upstream, "::nogroups");
//         (void) ngx_http_test_traffic_status_node_position_key(&upstream_server, 1);
//     }

//     buf = ngx_sprintf(buf, NGX_HTTP_test_TRAFFIC_STATUS_PROMETHEUS_FMT_UPSTREAM,
//                       &upstream, &upstream_server, vtsn->stat_in_bytes,
//                       &upstream, &upstream_server, vtsn->stat_out_bytes,
//                       &upstream, &upstream_server, vtsn->stat_1xx_counter,
//                       &upstream, &upstream_server, vtsn->stat_2xx_counter,
//                       &upstream, &upstream_server, vtsn->stat_3xx_counter,
//                       &upstream, &upstream_server, vtsn->stat_4xx_counter,
//                       &upstream, &upstream_server, vtsn->stat_5xx_counter,
//                       &upstream, &upstream_server, (double) vtsn->stat_request_time_counter / 1000,
//                       &upstream, &upstream_server,
//                       (double) ngx_http_test_traffic_status_node_time_queue_average(
//                           &vtsn->stat_request_times, vtscf->average_method,
//                           vtscf->average_period) / 1000,
//                       &upstream, &upstream_server, (double) vtsn->stat_upstream.response_time_counter / 1000,
//                       &upstream, &upstream_server,
//                       (double) ngx_http_test_traffic_status_node_time_queue_average(
//                           &vtsn->stat_upstream.response_times, vtscf->average_method,
//                           vtscf->average_period) / 1000);

    // /* histogram */
    // len = 2;

    // while (len--) {
    //     if (len > 0) {
    //         b = &vtsn->stat_request_buckets;
    //         time_counter = vtsn->stat_request_time_counter;
    //         ngx_str_set(&target, "request");

    //     } else {
    //         b = &vtsn->stat_upstream.response_buckets;
    //         time_counter = vtsn->stat_upstream.response_time_counter;
    //         ngx_str_set(&target, "response");
    //     }

    //     n = b->len;

    //     if (n > 0) {
    //         /* histogram:bucket */
    //         for (i = 0; i < n; i++) {
    //             buf = ngx_sprintf(buf,
    //                     NGX_HTTP_TEST_TRAFFIC_STATUS_PROMETHEUS_FMT_UPSTREAM_HISTOGRAM_BUCKET,
    //                     &target, &upstream, &upstream_server, (double) b->buckets[i].msec / 1000,
    //                     b->buckets[i].counter);
    //         }

    //         buf = ngx_sprintf(buf,
    //                 NGX_HTTP_TEST_TRAFFIC_STATUS_PROMETHEUS_FMT_UPSTREAM_HISTOGRAM_BUCKET_E,
    //                 &target, &upstream, &upstream_server, vtsn->stat_request_counter);

    //         /* histogram:sum */
    //         buf = ngx_sprintf(buf,
    //                 NGX_HTTP_TEST_TRAFFIC_STATUS_PROMETHEUS_FMT_UPSTREAM_HISTOGRAM_SUM,
    //                 &target, &upstream, &upstream_server, (double) time_counter / 1000);

    //         /* histogram:count */
    //         buf = ngx_sprintf(buf,
    //                 NGX_HTTP_TEST_TRAFFIC_STATUS_PROMETHEUS_FMT_UPSTREAM_HISTOGRAM_COUNT,
    //                 &target, &upstream, &upstream_server, vtsn->stat_request_counter);
    //     }

    // }

//     return buf;
// }


// u_char *
// ngx_http_test_traffic_status_display_prometheus_set_upstream(ngx_http_request_t *r,
//     u_char *buf, ngx_rbtree_node_t *node)
// {
//     ngx_str_t                              key, escaped_key;
//     ngx_http_test_traffic_status_ctx_t   *ctx;
//     ngx_http_test_traffic_status_node_t  *vtsn;

//     ctx = ngx_http_get_module_main_conf(r, ngx_http_test_traffic_status_module);

//     if (node != ctx->rbtree->sentinel) {
//         vtsn = (ngx_http_test_traffic_status_node_t *) &node->color;

//         if (vtsn->stat_upstream.type == NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_UG
//             || vtsn->stat_upstream.type == NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_UA)
//         {
//             key.data = vtsn->data;
//             key.len = vtsn->len;

//             ngx_http_test_traffic_status_escape_prometheus(r->pool, &escaped_key, key.data, key.len);
//             buf = ngx_http_test_traffic_status_display_prometheus_set_upstream_node(r, buf, &escaped_key, vtsn);
//         }

//         buf = ngx_http_test_traffic_status_display_prometheus_set_upstream(r, buf, node->left);
//         buf = ngx_http_test_traffic_status_display_prometheus_set_upstream(r, buf, node->right);
//     }

//     return buf;
// }


ngx_int_t
ngx_http_test_traffic_status_escape_prometheus(ngx_pool_t *pool, ngx_str_t *buf, u_char *p, size_t n)
{
    u_char  c, *pa, *pb, *last, *char_end;
    size_t  size;
    u_char  HEX_MAP[] = "0123456789ABCDEF";

    last = p + n;
    pa = p;
    size = 0;

    /* Find the first character that needs to be escaped */
    while (pa < last) {
        if isascii(*pa) {
            if (*pa == '"' || *pa == '\\' || *pa == '\n') {
                break;
            } else {
                pa++;
            }
        } else {
            char_end = pa;
            if (ngx_utf8_decode(&char_end, last - pa) > 0x10ffff) {
                break;
            } else {
                pa = char_end;
            }
        }
    }

    if (pa == last) {
        // no escapes required - return the original string
        buf->data = p;
        buf->len = n;
        return NGX_OK;
    }

    size = pa - p;

    /* Allocate enough space for the unescaped prefix and worst case for remainder */
    buf->data = ngx_pcalloc(pool, size + (n - size) * 5);
    if (buf->data == NULL) {
        /*
            Return the unescaped string up to the first special character 
            in case the caller does not handle the error.
        */
        buf->data = p;
        buf->len = size;
        return NGX_ERROR;
    }

    /* Copy `size` unescaped characters to start of destination. */
    pb = ngx_copy(buf->data, p, size);

    /* Individually copy remaining characters to destination, escaping as necessary */
    while (pa < last) {
        if (isascii(*pa)) {
            if (*pa == '"' || *pa == '\\') {
                *pb++ = '\\';
                *pb++ = *pa++;
                size += 2;
            } else if (*pa == '\n') {
                *pb++ = '\\';
                *pb++ = 'n';
                pa++;
                size += 2;
            } else {
                *pb++ = *pa++;
                size++;
            }
        } else {
            char_end = pa;
            if (ngx_utf8_decode(&char_end, last - pa) > 0x10ffff) {
                /* invalid UTF-8 - escape single char to allow resynchronization */
                c = *pa++;
                /* two slashes are required to be valid encoding for prometheus*/
                *pb++ = '\\';
                *pb++ = '\\';
                *pb++ = 'x';
                *pb++ = HEX_MAP[c >> 4];
                *pb++ = HEX_MAP[c & 0x0f];
                size += 5;
            } else {
                while (pa < char_end) {
                    *pb++ = *pa++;
                    size++;
                }
            }
        }
    }

    buf->len = size;
    return NGX_OK;
}




u_char *
ngx_http_test_traffic_status_display_prometheus_set(ngx_http_request_t *r,
    u_char *buf)
{
    ngx_str_t                                 escaped_key;
/*     u_char                                    *o, *s; */
    ngx_rbtree_node_t                         *node;
    ngx_http_test_traffic_status_ctx_t       *ctx;
    ngx_http_test_traffic_status_loc_conf_t  *vtscf;

    ctx = ngx_http_get_module_main_conf(r, ngx_http_test_traffic_status_module);

    vtscf = ngx_http_get_module_loc_conf(r, ngx_http_test_traffic_status_module);

    node = ctx->rbtree->root;

    /* init stats */
    ngx_memzero(&vtscf->stats, sizeof(vtscf->stats));
    ngx_http_test_traffic_status_node_time_queue_init(&vtscf->stats.stat_request_times);

    /* main & connections */
    buf = ngx_http_test_traffic_status_display_prometheus_set_main(r, buf);

    /* serverZones */
    buf = ngx_sprintf(buf, NGX_HTTP_TEST_TRAFFIC_STATUS_PROMETHEUS_FMT_SERVER_S);
// #if (NGX_HTTP_CACHE)
//     buf = ngx_sprintf(buf, NGX_HTTP_TEST_TRAFFIC_STATUS_PROMETHEUS_FMT_SERVER_CACHE_S);
// #endif
    buf = ngx_http_test_traffic_status_display_prometheus_set_server(r, buf, node);

    ngx_http_test_traffic_status_escape_prometheus(r->pool, &escaped_key, vtscf->sum_key.data, vtscf->sum_key.len);
    buf = ngx_http_test_traffic_status_display_prometheus_set_server_node(r, buf, &escaped_key, &vtscf->stats);
    
//     /* filterZones */
//     o = buf;

//     buf = ngx_sprintf(buf, NGX_HTTP_TEST_TRAFFIC_STATUS_PROMETHEUS_FMT_FILTER_S);
// // #if (NGX_HTTP_CACHE)
// //     buf = ngx_sprintf(buf, NGX_HTTP_TEST_TRAFFIC_STATUS_PROMETHEUS_FMT_FILTER_CACHE_S);
// // #endif

//     s = buf;

//     buf = ngx_http_test_traffic_status_display_prometheus_set_filter(r, buf, node);

//     if (s == buf) {
//         buf = o;
//     }

//     /* upstreamZones */
//     o = buf;

//     buf = ngx_sprintf(buf, NGX_HTTP_TEST_TRAFFIC_STATUS_PROMETHEUS_FMT_UPSTREAM_S);

//     s = buf;

//     buf = ngx_http_test_traffic_status_display_prometheus_set_upstream(r, buf, node);

//     if (s == buf) {
//         buf = o;
//     }


    return buf;
}