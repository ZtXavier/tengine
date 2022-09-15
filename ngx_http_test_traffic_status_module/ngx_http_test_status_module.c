#include"ngx_http_test_status_module.h"
#include"ngx_http_test_status_module_html.h"
#include"ngx_http_test_status_module_prome.h"
#include"ngx_http_test_status_module_variable.h"
#include"ngx_http_test_status_module_shm.h"

ngx_int_t ngx_http_test_traffic_status_display_get_upstream_nelts(ngx_http_request_t *r);
ngx_int_t ngx_http_test_traffic_status_display_get_size(ngx_http_request_t *r,ngx_int_t format);
static ngx_int_t ngx_http_test_traffic_status_handler(ngx_http_request_t *r);
static void ngx_http_test_traffic_status_rbtree_insert_value(
    ngx_rbtree_node_t *temp, ngx_rbtree_node_t *node,
    ngx_rbtree_node_t *sentinel);
static ngx_int_t ngx_http_test_traffic_status_init_zone(
    ngx_shm_zone_t *shm_zone, void *data);
static char *ngx_http_test_traffic_status_zone(ngx_conf_t *cf,
    ngx_command_t *cmd, void *conf);


static ngx_int_t ngx_http_test_traffic_status_preconfiguration(ngx_conf_t *cf);
static ngx_int_t ngx_http_test_traffic_status_init(ngx_conf_t *cf);
static void *ngx_http_test_traffic_status_create_main_conf(ngx_conf_t *cf);
static char *ngx_http_test_traffic_status_init_main_conf(ngx_conf_t *cf,
    void *conf);
static void *ngx_http_test_traffic_status_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_test_traffic_status_merge_loc_conf(ngx_conf_t *cf,
    void *parent, void *child);



static ngx_int_t ngx_http_test_traffic_status_display_handler_default(ngx_http_request_t *r);
char *ngx_http_test_traffic_status_display(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t  ngx_http_test_traffic_status_display_handler(ngx_http_request_t *r);

static ngx_conf_enum_t  ngx_http_test_traffic_status_display_format[] = {
    { ngx_string("html"), 
    NGX_HTTP_TEST_STATUS_FORMAT_HTML },

    { ngx_string("prometheus"), 
    NGX_HTTP_TEST_STATUS_FORMAT_PROMETHEUS },

    { ngx_null_string, 
    0 }
};


static ngx_command_t ngx_http_test_traffic_status_commands[] = {

    { ngx_string("test_traffic_status"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_test_traffic_status_loc_conf_t, enable),
      NULL },

    { ngx_string("test_traffic_status_zone"),
      NGX_HTTP_MAIN_CONF|NGX_CONF_NOARGS|NGX_CONF_TAKE1,
      ngx_http_test_traffic_status_zone,
      0,
      0,
      NULL },

    { ngx_string("test_traffic_status_display"),
      NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS|NGX_CONF_TAKE1,
      ngx_http_test_traffic_status_display,
      0,
      0,
      NULL },

    { ngx_string("test_traffic_status_display_format"),
      NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_enum_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_test_traffic_status_loc_conf_t, format),
      &ngx_http_test_traffic_status_display_format },

    ngx_null_command
};

static ngx_http_module_t ngx_http_test_traffic_status_module_ctx = {
    ngx_http_test_traffic_status_preconfiguration, /* preconfiguration */
    ngx_http_test_traffic_status_init,             /* postconfiguration */

    ngx_http_test_traffic_status_create_main_conf, /* create main configuration */
    ngx_http_test_traffic_status_init_main_conf,   /* init main configuration */

    NULL,                                           /* create server configuration */
    NULL,                                           /* merge server configuration */

    ngx_http_test_traffic_status_create_loc_conf,  /* create location configuration */
    ngx_http_test_traffic_status_merge_loc_conf,   /* merge location configuration */
};


ngx_module_t ngx_http_test_traffic_status_module = {
    NGX_MODULE_V1,
    &ngx_http_test_traffic_status_module_ctx,   /* module context */
    ngx_http_test_traffic_status_commands,    /* module directives */
    NGX_HTTP_MODULE,                               /* module type */
    NULL,                                        /* init master */
    NULL,                                        /* init module */
    NULL,                                        /* init process */
    NULL,                                        /* init thread */
    NULL,                                        /* exit thread */
    NULL,                                       /* exit process */
    NULL,                                        /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_test_traffic_status_preconfiguration(ngx_conf_t *cf)
{
    return ngx_http_test_traffic_status_add_variables(cf);
}


static ngx_int_t
ngx_http_test_traffic_status_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, cf->log, 0,
                   "http vts init");

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);


    // /* set handler */
    // h = ngx_array_push(&cmcf->phases[NGX_HTTP_ACCESS_PHASE].handlers);
    // if (h == NULL) {
    //     return NGX_ERROR;
    // }

    // *h = ngx_http_test_traffic_status_set_handler;

    /* vts handler */
    h = ngx_array_push(&cmcf->phases[NGX_HTTP_LOG_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_test_traffic_status_handler;

    return NGX_OK;
}



static char *
ngx_http_test_traffic_status_init_main_conf(ngx_conf_t *cf, void *conf)
{
    ngx_http_test_traffic_status_ctx_t  *ctx = conf;

    ngx_int_t                                  rc;
    ngx_http_test_traffic_status_loc_conf_t  *vtscf;

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, cf->log, 0,
                   "http vts init main conf");

    vtscf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_test_traffic_status_module);

    if (vtscf->filter_check_duplicate != 0) {
        rc = ngx_http_test_traffic_status_filter_unique(cf->pool, &ctx->filter_keys);
        if (rc != NGX_OK) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "init_main_conf::filter_unique() failed");
            return NGX_CONF_ERROR;
        }
    }

    if (vtscf->limit_check_duplicate != 0) {
        rc = ngx_http_test_traffic_status_limit_traffic_unique(cf->pool, &ctx->limit_traffics);
        if (rc != NGX_OK) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "init_main_conf::limit_traffic_unique(server) failed");
            return NGX_CONF_ERROR;
        }

        rc = ngx_http_test_traffic_status_limit_traffic_unique(cf->pool,
                                                                &ctx->limit_filter_traffics);
        if (rc != NGX_OK) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "init_main_conf::limit_traffic_unique(filter) failed");
            return NGX_CONF_ERROR;
        }
    }

    ngx_conf_init_uint_value(ctx->filter_max_node, 0);
    ngx_conf_init_value(ctx->enable, 0);
    ngx_conf_init_value(ctx->filter_check_duplicate, vtscf->filter_check_duplicate);
    ngx_conf_init_value(ctx->limit_check_duplicate, vtscf->limit_check_duplicate);

    return NGX_CONF_OK;
}




static void *
ngx_http_test_traffic_status_create_main_conf(ngx_conf_t *cf)
{
    ngx_http_test_traffic_status_ctx_t  *ctx;

    ctx = ngx_pcalloc(cf->pool, sizeof(ngx_http_test_traffic_status_ctx_t));
    if (ctx == NULL) {
        return NULL;
    }

    ctx->filter_max_node = NGX_CONF_UNSET_UINT;
    ctx->enable = NGX_CONF_UNSET;
    ctx->filter_check_duplicate = NGX_CONF_UNSET;
    ctx->limit_check_duplicate = NGX_CONF_UNSET;


    return ctx;
}


static void *
ngx_http_test_traffic_status_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_test_traffic_status_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_test_traffic_status_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }
    conf->shm_zone = NGX_CONF_UNSET_PTR;
    conf->enable = NGX_CONF_UNSET;
    
    conf->start_msec = ngx_http_test_traffic_status_current_msec();

    return conf;
}


static char *
ngx_http_test_traffic_status_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_test_traffic_status_loc_conf_t *prev = parent;
    ngx_http_test_traffic_status_loc_conf_t *conf = child;

   /*  ngx_int_t                             rc; */
    ngx_str_t                             name;
    ngx_shm_zone_t                       *shm_zone;
    ngx_http_test_traffic_status_ctx_t  *ctx;

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, cf->log, 0,
                   "http vts merge loc conf");

    ctx = ngx_http_conf_get_module_main_conf(cf, ngx_http_test_traffic_status_module);

    if (!ctx->enable) {
        return NGX_CONF_OK;
    }

    if (conf->filter_keys == NULL) {
        conf->filter_keys = prev->filter_keys;

    } else {
        if (conf->filter_check_duplicate == NGX_CONF_UNSET) {
            conf->filter_check_duplicate = ctx->filter_check_duplicate;
        }
       /*  if (conf->filter_check_duplicate != 0) {
            rc = ngx_http_test_traffic_status_filter_unique(cf->pool, &conf->filter_keys);
            if (rc != NGX_OK) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "mere_loc_conf::filter_unique() failed");
                return NGX_CONF_ERROR;
            }
        } */
    }

    if (conf->limit_traffics == NULL) {
        conf->limit_traffics = prev->limit_traffics;

    } else {
        if (conf->limit_check_duplicate == NGX_CONF_UNSET) {
            conf->limit_check_duplicate = ctx->limit_check_duplicate;
        }

       /*  if (conf->limit_check_duplicate != 0) {
            rc = ngx_http_test_traffic_status_limit_traffic_unique(cf->pool,
                                                                    &conf->limit_traffics);
            if (rc != NGX_OK) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "mere_loc_conf::limit_traffic_unique(server) failed");
                return NGX_CONF_ERROR;
            }
        } */
    }

    if (conf->limit_filter_traffics == NULL) {
        conf->limit_filter_traffics = prev->limit_filter_traffics;

    } else {
        if (conf->limit_check_duplicate == NGX_CONF_UNSET) {
            conf->limit_check_duplicate = ctx->limit_check_duplicate;
        }

        /* if (conf->limit_check_duplicate != 0) {
            rc = ngx_http_test_traffic_status_limit_traffic_unique(cf->pool,
                                                                    &conf->limit_filter_traffics);
            if (rc != NGX_OK) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "mere_loc_conf::limit_traffic_unique(filter) failed");
                return NGX_CONF_ERROR;
            }
        } */
    }

    ngx_conf_merge_ptr_value(conf->shm_zone, prev->shm_zone, NULL);
    ngx_conf_merge_value(conf->enable, prev->enable, 1);
    name = ctx->shm_name;

    shm_zone = ngx_shared_memory_add(cf, &name, 0,
                                     &ngx_http_test_traffic_status_module);
    if (shm_zone == NULL) {
        return NGX_CONF_ERROR;
    }

    conf->shm_zone = shm_zone;
    conf->shm_name = name;

    return NGX_CONF_OK;
}




static ngx_int_t
ngx_http_test_traffic_status_handler(ngx_http_request_t *r)
{
    ngx_int_t                                  rc;
    ngx_http_test_traffic_status_ctx_t       *ctx;
    ngx_http_test_traffic_status_loc_conf_t  *vtscf;

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http vts handler");

    ctx = ngx_http_get_module_main_conf(r, ngx_http_test_traffic_status_module);
    vtscf = ngx_http_get_module_loc_conf(r, ngx_http_test_traffic_status_module);

    if (!ctx->enable || !vtscf->enable || vtscf->bypass_stats) {
        return NGX_DECLINED;
    }
    if (vtscf->shm_zone == NULL) {
        return NGX_DECLINED;
    }

    rc = ngx_http_test_traffic_status_shm_add_server(r);
    if (rc != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "handler::shm_add_server() failed");
    }

//     rc = ngx_http_test_traffic_status_shm_add_upstream(r);
//     if (rc != NGX_OK) {
//         ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
//                       "handler::shm_add_upstream() failed");
//     }

//     rc = ngx_http_test_traffic_status_shm_add_filter(r);
//     if (rc != NGX_OK) {
//         ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
//                       "handler::shm_add_filter() failed");
//     }

// #if (NGX_HTTP_CACHE)
//     rc = ngx_http_test_traffic_status_shm_add_cache(r);
//     if (rc != NGX_OK) {
//         ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
//                       "handler::shm_add_cache() failed");
//     }
// #endif

    return NGX_DECLINED;
}



ngx_msec_t
ngx_http_test_traffic_status_current_msec(void)
{
    time_t           sec;
    ngx_uint_t       msec;
    struct timeval   tv;

    ngx_gettimeofday(&tv);

    sec = tv.tv_sec;
    msec = tv.tv_usec / 1000;

    return (ngx_msec_t) sec * 1000 + msec;
}


ngx_msec_int_t
ngx_http_test_traffic_status_request_time(ngx_http_request_t *r)
{
    ngx_time_t      *tp;
    ngx_msec_int_t   ms;

    tp = ngx_timeofday();

    ms = (ngx_msec_int_t)
             ((tp->sec - r->start_sec) * 1000 + (tp->msec - r->start_msec));
    return ngx_max(ms, 0);
}



ngx_msec_int_t
ngx_http_test_traffic_status_upstream_response_time(ngx_http_request_t *r)
{
    ngx_uint_t                  i;
    ngx_msec_int_t              ms;
    ngx_http_upstream_state_t  *state;

    state = r->upstream_states->elts;

    i = 0;
    ms = 0;
    for ( ;; ) {
        if (state[i].status) {

#if !defined(nginx_version) || nginx_version < 1009001
            ms += (ngx_msec_int_t)
                  (state[i].response_sec * 1000 + state[i].response_msec);
#else
            ms += state[i].response_time;
#endif

        }
        if (++i == r->upstream_states->nelts) {
            break;
        }
    }
    return ngx_max(ms, 0);
}


static void
ngx_http_test_traffic_status_rbtree_insert_value(ngx_rbtree_node_t *temp,
    ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel)
{
    ngx_rbtree_node_t                     **p;
    ngx_http_test_traffic_status_node_t   *vtsn, *vtsnt;

    for ( ;; ) {

        if (node->key < temp->key) {

            p = &temp->left;

        } else if (node->key > temp->key) {

            p = &temp->right;

        } else { /* node->key == temp->key */

            vtsn = (ngx_http_test_traffic_status_node_t *) &node->color;
            vtsnt = (ngx_http_test_traffic_status_node_t *) &temp->color;

            p = (ngx_memn2cmp(vtsn->data, vtsnt->data, vtsn->len, vtsnt->len) < 0)
                ? &temp->left
                : &temp->right;
        }
        if (*p == sentinel) {
            break;
        }

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    ngx_rbt_red(node);
}



static ngx_int_t
ngx_http_test_traffic_status_init_zone(ngx_shm_zone_t *shm_zone, void *data)
{
    ngx_http_test_traffic_status_ctx_t  *octx = data;

    size_t                                len;
    ngx_slab_pool_t                      *shpool;
    ngx_rbtree_node_t                    *sentinel;
    ngx_http_test_traffic_status_ctx_t  *ctx;

    ctx = shm_zone->data;

    if (octx) {
        ctx->rbtree = octx->rbtree;
        return NGX_OK;
    }

    shpool = (ngx_slab_pool_t *) shm_zone->shm.addr;

    if (shm_zone->shm.exists) {
        ctx->rbtree = shpool->data;
        return NGX_OK;
    }

    ctx->rbtree = ngx_slab_alloc(shpool, sizeof(ngx_rbtree_t));
    if (ctx->rbtree == NULL) {
        return NGX_ERROR;
    }

    shpool->data = ctx->rbtree;

    sentinel = ngx_slab_alloc(shpool, sizeof(ngx_rbtree_node_t));
    if (sentinel == NULL) {
        return NGX_ERROR;
    }

    ngx_rbtree_init(ctx->rbtree, sentinel,
                    ngx_http_test_traffic_status_rbtree_insert_value);

    len = sizeof(" in test_traffic_status_zone \"\"") + shm_zone->shm.name.len;

    shpool->log_ctx = ngx_slab_alloc(shpool, len);
    if (shpool->log_ctx == NULL) {
        return NGX_ERROR;
    }

    ngx_sprintf(shpool->log_ctx, " in test_traffic_status_zone \"%V\"%Z",
                &shm_zone->shm.name);

    return NGX_OK;
}


static char *
ngx_http_test_traffic_status_zone(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    u_char                               *p;
    ssize_t                               size;
    ngx_str_t                            *value, name, s;
    ngx_uint_t                            i;
    ngx_shm_zone_t                       *shm_zone;
    ngx_http_test_traffic_status_ctx_t  *ctx;

    value = cf->args->elts;

    ctx = ngx_http_conf_get_module_main_conf(cf, ngx_http_test_traffic_status_module);
    if (ctx == NULL) {
        return NGX_CONF_ERROR;
    }

    ctx->enable = 1;

    ngx_str_set(&name, NGX_HTTP_TEST_TRAFFIC_STATUS_DEFAULT_SHM_NAME);

    size = NGX_HTTP_TEST_TRAFFIC_STATUS_DEFAULT_SHM_SIZE;

    for (i = 1; i < cf->args->nelts; i++) {
        if (ngx_strncmp(value[i].data, "shared:", 7) == 0) {

            name.data = value[i].data + 7;

            p = (u_char *) ngx_strchr(name.data, ':');
            if (p == NULL) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "invalid shared size \"%V\"", &value[i]);
                return NGX_CONF_ERROR;
            }

            name.len = p - name.data;

            s.data = p + 1;
            s.len = value[i].data + value[i].len - s.data;

            size = ngx_parse_size(&s);
            if (size == NGX_ERROR) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "invalid shared size \"%V\"", &value[i]);
                return NGX_CONF_ERROR;
            }

            if (size < (ssize_t) (8 * ngx_pagesize)) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "shared \"%V\" is too small", &value[i]);
                return NGX_CONF_ERROR;
            }

            continue;
        }

        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "invalid parameter \"%V\"", &value[i]);
        return NGX_CONF_ERROR;
    }

    shm_zone = ngx_shared_memory_add(cf, &name, size,
                                     &ngx_http_test_traffic_status_module);
    if (shm_zone == NULL) {
        return NGX_CONF_ERROR;
    }

    if (shm_zone->data) {
        ctx = shm_zone->data;

        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "test_traffic_status: \"%V\" is already bound to key",
                           &name);

        return NGX_CONF_ERROR;
    }

    ctx->shm_zone = shm_zone;
    ctx->shm_name = name;
    ctx->shm_size = size;
    shm_zone->init = ngx_http_test_traffic_status_init_zone;
    shm_zone->data = ctx;

    return NGX_CONF_OK;
}


char *
ngx_http_test_traffic_status_display(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_test_traffic_status_display_handler;

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_test_traffic_status_display_handler(ngx_http_request_t *r)
{
/*     size_t                                     len;
    u_char                                    *p; */
    ngx_int_t                                  rc;
    ngx_http_test_traffic_status_ctx_t       *ctx;

    ctx = ngx_http_get_module_main_conf(r, ngx_http_test_traffic_status_module);

    if (!ctx->enable) {
        return NGX_HTTP_NOT_IMPLEMENTED;
    }

    if (r->method != NGX_HTTP_GET && r->method != NGX_HTTP_HEAD) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    // len = 0;

    // p = (u_char *) ngx_strlchr(r->uri.data, r->uri.data + r->uri.len, '/');

    // if (p) {
    //     p = (u_char *) ngx_strlchr(p + 1, r->uri.data + r->uri.len, '/');
    //     len = r->uri.len - (p - r->uri.data);
    // }

    // /* control processing handler */
    // if (p && len >= sizeof("/control") - 1) {
    //     p = r->uri.data + r->uri.len - sizeof("/control") + 1;
    //     if (ngx_strncasecmp(p, (u_char *) "/control", sizeof("/control") - 1) == 0) {
    //         rc = ngx_http_test_traffic_status_display_handler_control(r);
    //         goto done;
    //     }
    // }

    /* default processing handler */
    rc = ngx_http_test_traffic_status_display_handler_default(r);

// done:

    return rc;
}



static ngx_int_t
ngx_http_test_traffic_status_display_handler_default(ngx_http_request_t *r)
{
    size_t                                     len;
    u_char                                    *o, *s, *p;
    ngx_str_t                                  uri, euri, type;
    ngx_int_t                                  size, format, rc;
    ngx_buf_t                                 *b;
    ngx_chain_t                                out;
    ngx_slab_pool_t                           *shpool;
    ngx_http_test_traffic_status_ctx_t       *ctx;
    ngx_http_test_traffic_status_loc_conf_t  *vtscf;

    ctx = ngx_http_get_module_main_conf(r, ngx_http_test_traffic_status_module);

    vtscf = ngx_http_get_module_loc_conf(r, ngx_http_test_traffic_status_module);

    if (!ctx->enable) {
        return NGX_HTTP_NOT_IMPLEMENTED;
    }

    if (r->method != NGX_HTTP_GET && r->method != NGX_HTTP_HEAD) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    uri = r->uri;

    format = NGX_HTTP_TEST_STATUS_FORMAT_NONE;

    if (uri.len == 1) {
        if (ngx_strncmp(uri.data, "/", 1) == 0) {
            uri.len = 0;
        }
    }

    o = (u_char *) r->uri.data;
    s = o;

    len = r->uri.len;

    while(sizeof("/format/type") - 1 <= len) {
        if (ngx_strncasecmp(s, (u_char *) "/format/", sizeof("/format/") - 1) == 0) {
            uri.data = o;
            uri.len = (o == s) ? 0 : (size_t) (s - o);

            s += sizeof("/format/") - 1;

            if (ngx_strncasecmp(s, (u_char *) "html", sizeof("html") - 1) == 0) {
                format = NGX_HTTP_TEST_STATUS_FORMAT_HTML;

            } else if (ngx_strncasecmp(s, (u_char *) "prometheus", sizeof("prometheus") - 1) == 0) {
                format = NGX_HTTP_TEST_STATUS_FORMAT_PROMETHEUS;

            } else {
                s -= 2;
            }

            if (format != NGX_HTTP_TEST_STATUS_FORMAT_NONE) {
                break;
            }
        }

        if ((s = (u_char *) ngx_strchr(++s, '/')) == NULL) {
            break;
        }

        if (r->uri.len <= (size_t) (s - o)) {
            break;
        }

        len = r->uri.len - (size_t) (s - o);
    }

    format = (format == NGX_HTTP_TEST_STATUS_FORMAT_NONE) ? vtscf->format : format;

    rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK) {
        return rc;
    }

    if (format == NGX_HTTP_TEST_STATUS_FORMAT_PROMETHEUS) {
        ngx_str_set(&type, "text/plain");

    } else {
        ngx_str_set(&type, "text/html");
    }

    r->headers_out.content_type_len = type.len;
    r->headers_out.content_type = type;

    if (r->method == NGX_HTTP_HEAD) {
        r->headers_out.status = NGX_HTTP_OK;

        rc = ngx_http_send_header(r);

        if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
            return rc;
        }
    }

    size = ngx_http_test_traffic_status_display_get_size(r, format);
    if (size == NGX_ERROR) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "display_handler_default::display_get_size() failed");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    b = ngx_create_temp_buf(r->pool, size);
    if (b == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "display_handler_default::ngx_create_temp_buf() failed");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    if (format == NGX_HTTP_TEST_STATUS_FORMAT_PROMETHEUS) {
        shpool = (ngx_slab_pool_t *) vtscf->shm_zone->shm.addr;
        ngx_shmtx_lock(&shpool->mutex);
        b->last = ngx_http_test_traffic_status_display_prometheus_set(r, b->last);
        ngx_shmtx_unlock(&shpool->mutex);

        if (b->last == b->pos) {
            b->last = ngx_sprintf(b->last, "#");
        }

    }
    else {
        euri = uri;
        len = ngx_escape_html(NULL, uri.data, uri.len);

        if (len) {
            p = ngx_pnalloc(r->pool, uri.len + len);
            if (p == NULL) {
                ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                              "display_handler_default::ngx_pnalloc() failed");
                return NGX_HTTP_INTERNAL_SERVER_ERROR;
            }

            (void) ngx_escape_html(p, uri.data, uri.len);
            euri.data = p;
            euri.len = uri.len + len;
        }

        b->last = ngx_sprintf(b->last, NGX_HTTP_TEST_STATUS_HTML_DATA, &euri, &euri);
    }

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = b->last - b->pos;

    b->last_buf = (r == r->main) ? 1 : 0; /* if subrequest 0 else 1 */
    b->last_in_chain = 1;

    out.buf = b;
    out.next = NULL;

    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    return ngx_http_output_filter(r, &out);
}

// display--
ngx_int_t
ngx_http_test_traffic_status_display_get_size(ngx_http_request_t *r,
    ngx_int_t format)
{
    ngx_uint_t                                 size, un;
    ngx_http_test_traffic_status_shm_info_t  *shm_info;

    shm_info = ngx_pcalloc(r->pool, sizeof(ngx_http_test_traffic_status_shm_info_t));
    if (shm_info == NULL) {
        return NGX_ERROR;
    }

    ngx_http_test_traffic_status_shm_info(r, shm_info);

    /* allocate memory for the upstream groups even if upstream node not exists */
    un = shm_info->used_node
         + (ngx_uint_t) ngx_http_test_traffic_status_display_get_upstream_nelts(r);

    size = 0;

    switch (format) {

    case NGX_HTTP_TEST_STATUS_FORMAT_PROMETHEUS:
        size = sizeof(ngx_http_test_traffic_status_node_t) / NGX_PTR_SIZE
               * NGX_ATOMIC_T_LEN * un  /* values size */
               + (un * 1024)            /* names  size */
               + 4096;                  /* main   size */
        break;

    case NGX_HTTP_TEST_STATUS_FORMAT_HTML:
        size = sizeof(NGX_HTTP_TEST_STATUS_HTML_DATA) + ngx_pagesize;
        break;
    }

    if (size <= 0) {
        size = shm_info->max_size;
    }

    ngx_log_debug3(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "vts::display_get_size(): size[%ui] used_size[%ui], used_node[%ui]",
                   size, shm_info->used_size, shm_info->used_node);

    return size;
}


ngx_int_t
ngx_http_test_traffic_status_display_get_upstream_nelts(ngx_http_request_t *r)
{
    ngx_uint_t                      i, j, n;
    ngx_http_upstream_server_t     *us;
#if (NGX_HTTP_UPSTREAM_ZONE)
    ngx_http_upstream_rr_peer_t    *peer;
    ngx_http_upstream_rr_peers_t   *peers;
#endif
    ngx_http_upstream_srv_conf_t   *uscf, **uscfp;
    ngx_http_upstream_main_conf_t  *umcf;

    umcf = ngx_http_get_module_main_conf(r, ngx_http_upstream_module);
    uscfp = umcf->upstreams.elts;

    for (i = 0, j = 0, n = 0; i < umcf->upstreams.nelts; i++) {

        uscf = uscfp[i];

        /* groups */
        if (uscf->servers && !uscf->port) {
            us = uscf->servers->elts;

#if (NGX_HTTP_UPSTREAM_ZONE)
            if (uscf->shm_zone == NULL) {
                goto not_supported;
            }

            peers = uscf->peer.data;

            ngx_http_upstream_rr_peers_rlock(peers);

            for (peer = peers->peer; peer; peer = peer->next) {
                n++;
            }

            ngx_http_upstream_rr_peers_unlock(peers);

not_supported:

#endif

            for (j = 0; j < uscf->servers->nelts; j++) {
                n += us[j].naddrs;
            }
        }
    }

    return n;
}




