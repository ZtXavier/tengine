#include<ngx_http_test_status_module.h>

static void *
ngx_http_new_status_create_main_conf(ngx_conf_t *cf){
    ngx_http_new_status_main_conf_t *nmcf;
    nmcf = ngx_pcalloc(cf->pool,sizeof(ngx_http_new_status_main_conf_t));

    if(nmcf == NULL){
        return NULL;
    }

    if(ngx_arrary_init(&nmcf->zones,cf->pool,4,sizeof(ngx_http__status_main_conf_t*)) != NGX_OK){
        return NULL;
    }

    nmcf->interval = NGX_CONF_UNSET_MSEC;
    nmcf->lock_time = NGX_CONF_UNSET;

    return nmcf;
}


static char *
ngx_http_new_status_init_main_conf(ngx_conf_t *cf, void *conf)
{
    ngx_http_new_status_main_conf_t *nmcf = conf;

    ngx_conf_init_msec_value(nmcf->interval, 3000);
    ngx_conf_init_value(nmcf->lock_time, 10);

    return NGX_CONF_OK;
}


static void *
ngx_http_new_status_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_new_status_loc_conf_t *nlcf;

    nlcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_new_status_loc_conf_t));
    if (nlcf == NULL) {
        return NULL;
    }

    nlcf->parent = NGX_CONF_UNSET_PTR;

    return nlcf;
}




static char *
ngx_http_new_status_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_new_status_loc_conf_t *prev = parent;
    ngx_http_new_status_loc_conf_t *conf = child;
    ngx_http_new_status_loc_conf_t *nlcf;

    if (conf->parent == NGX_CONF_UNSET_PTR){
        nlcf = prev;

        if (nlcf->parent == NGX_CONF_UNSET_PTR) {
            nlcf->parent = NULL;
        } else {
            while (nlcf->parent && nlcf->req_zones.nelts == 0) {
                nlcf = nlcf->parent;
            }
        }
        conf->parent = nlcf->req_zones.nelts ? nlcf : NULL;
    }

    return NGX_CONF_OK;
}




// 该函数是作为监控指标的处理回调函数
static ngx_int_t
ngx_http_new_status_handler(ngx_http_request_t *r){
    // 基本的数据类型
     size_t                              size, item_size;
    u_char                              long_num, full_info, clear_status;
    ngx_int_t                           rc;
    ngx_buf_t                          *b;
    ngx_uint_t                          i;
    ngx_array_t                         items;
    ngx_queue_t                        *q;
    ngx_chain_t                         out;

    // 下面需要统计的指标





    
}





static ngx_int_t
ngx_http_new_status_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt             *ret;
    ngx_http_core_main_conf_t    *cmcf;
    ngx_http_new_status_main_conf_t   *nmcf;

    nmcf = ngx_http_conf_get_module_main_conf(cf,ngx_http_new_status_module);

    if(nmcf->zones.nelts == 0){
        return NGX_OK;
    }
    cmcf = ngx_http_conf_get_module_main_conf(cf,ngx_http_core_module);

    ret = ngx_arrary_push(&cmcf->phases[NGX_HTTP_PREACCESS_PHASE].handlers);
    if(ret == NULL){
        return NGX_ERROR;
    }

    *ret = ngx_http_new_status_handler;

    
    return NGX_OK;
}

