#include"ngx_http_test_status_module.h"





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

    ngx_int_t                             rc;
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
        if (conf->filter_check_duplicate != 0) {
            rc = ngx_http_test_traffic_status_filter_unique(cf->pool, &conf->filter_keys);
            if (rc != NGX_OK) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "mere_loc_conf::filter_unique() failed");
                return NGX_CONF_ERROR;
            }
        }
    }

    if (conf->limit_traffics == NULL) {
        conf->limit_traffics = prev->limit_traffics;

    } else {
        if (conf->limit_check_duplicate == NGX_CONF_UNSET) {
            conf->limit_check_duplicate = ctx->limit_check_duplicate;
        }

        if (conf->limit_check_duplicate != 0) {
            rc = ngx_http_test_traffic_status_limit_traffic_unique(cf->pool,
                                                                    &conf->limit_traffics);
            if (rc != NGX_OK) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "mere_loc_conf::limit_traffic_unique(server) failed");
                return NGX_CONF_ERROR;
            }
        }
    }

    if (conf->limit_filter_traffics == NULL) {
        conf->limit_filter_traffics = prev->limit_filter_traffics;

    } else {
        if (conf->limit_check_duplicate == NGX_CONF_UNSET) {
            conf->limit_check_duplicate = ctx->limit_check_duplicate;
        }

        if (conf->limit_check_duplicate != 0) {
            rc = ngx_http_test_traffic_status_limit_traffic_unique(cf->pool,
                                                                    &conf->limit_filter_traffics);
            if (rc != NGX_OK) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "mere_loc_conf::limit_traffic_unique(filter) failed");
                return NGX_CONF_ERROR;
            }
        }
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

    rc = ngx_http_test_traffic_status_shm_add_upstream(r);
    if (rc != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "handler::shm_add_upstream() failed");
    }

    rc = ngx_http_test_traffic_status_shm_add_filter(r);
    if (rc != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "handler::shm_add_filter() failed");
    }

#if (NGX_HTTP_CACHE)
    rc = ngx_http_test_traffic_status_shm_add_cache(r);
    if (rc != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "handler::shm_add_cache() failed");
    }
#endif

    return NGX_DECLINED;
}