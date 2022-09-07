#include<ngx_http_test_status_module.h>

static void *
ngx_http_new_status_create_main_conf(ngx_conf_t *cf){
    ngx_http_new_status_main_conf_t *nmcf;
    nmcf = ngx_pcalloc(cf->pool,sizeof(ngx_http_new_status_main_conf_t));

    if(nmcf == NULL){
        return NULL;
    }

    if(ngx_arrary_init(&nmcf->zones,cf->pool,4,sizeof(ngx_http_status_main_conf_t*)) != NGX_OK){
        return NULL;
    }

    nmcf->interval = NGX_CONF_UNSET_MSEC;
    nmcf->lock_time = NGX_CONF_UNSET;

    return nmcf;
}


static char *
ngx_http_new_status_init_main_conf(ngx_conf_t *cf, void *conf){
    ngx_http_new_status_main_conf_t *nmcf = conf;

    ngx_conf_init_msec_value(nmcf->interval, 3000);
    ngx_conf_init_value(nmcf->lock_time, 10);

    return NGX_CONF_OK;
}


static void *
ngx_http_new_status_create_loc_conf(ngx_conf_t *cf){
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


static void
ngx_http_new_status_rbtree_insert_value(ngx_rbtree_node_t *temp,
        ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel)
{
    ngx_rbtree_node_t               **p;
    ngx_http_new_status_node_t      *cn, *cnt;

    for ( ;; ) {

        if (node->key < temp->key) {

            p = &temp->left;

        } else if (node->key > temp->key) {

            p = &temp->right;

        } else { /* node->key == temp->key */

            cn = (ngx_http_new_status_node_t *) node;
            cnt = (ngx_http_new_status_node_t *) temp;

            p = (ngx_memn2cmp(cn->key, cnt->key, cn->len, cnt->len) < 0)
                ? &temp->left : &temp->right;
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
ngx_http_new_status_init_zone(ngx_shm_zone_t *shm_zone,void *data){
    size_t      len;
    ngx_http_new_status_zone_t  *ctx = shm_zone->data;
    ngx_http_new_status_zone_t *octx = data;

    if(octx != NULL) {
        if(ngx_strcmp(&octx->key.value,&ctx->key.value) != 0) {
            ngx_log_error(NGX_LOG_EMERG,shm_zone->shm.log,0,
            "status_new \" %V \"  ctx.v = \" %V \" octx.v = \" %V \" error",
            &shm_zone->shm.name,&ctx->key.value,&octx->key.value);
            return NGX_ERROR;
        }
        ctx->sh = octx->sh;
        ctx->shpool = octx->shpool;

        return NGX_OK;
    }

    ctx->shpool = (ngx_slab_pool_t *) shm_zone->shm.addr;
    if(shm_zone->shm.exists) {
        ctx->sh = ctx->shpool->data;
        return NGX_OK;
    }


    // 申请大内存(page)
    ctx->sh = ngx_slab_alloc(ctx->shpool,sizeof(ngx_http_new_status_sh_t));
    if(ctx->sh == NULL) {
        return NGX_ERROR;
    }

    ctx->shpool->data = ctx->sh;

    ngx_rbtree_init(&ctx->sh->rbtree,&ctx->sh->sentinel,
    ngx_http_new_status_rbtree_insert_value); 

    ngx_queue_init(&ctx->sh->queue);

    ctx->sh->expire_lock = 0;

    len = sizeof("in status zone\"\"") + shm_zone->shm.name.len;

    ctx->shpool->log_ctx = ngx_slab_alloc(ctx->shpool,len);
    if(ctx->shpool->log_ctx == NULL) {
        return NGX_ERROR;
    }

    ngx_sprintf(ctx->shpool->log_ctx,"in status zone \"%V\"%Z",&shm_zone->shm.name);

    return NGX_OK;
}


static char *
ngx_http_new_status(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_new_status_loc_conf_t *rlcf = conf;

    ngx_str_t                      *value;
    ngx_uint_t                      i, m;
    ngx_shm_zone_t                 *shm_zone, **zones, **pzone;

    value = cf->args->elts;

    zones = rlcf->req_zones.elts;

    for (i = 1; i < cf->args->nelts; i++){
        if (value[i].data[0] == '@') {
            rlcf->parent = NULL;

            if (value[i].len == 1) {
                continue;
            }

            value[i].data ++;
            value[i].len --;
        }

        shm_zone = ngx_shared_memory_add(cf, &value[i], 0,
                &ngx_http_new_status_module);
        if (shm_zone == NULL) {
            return NGX_CONF_ERROR;
        }

        if (zones == NULL) {
            if (ngx_array_init(&rlcf->req_zones, cf->pool, 2, sizeof(ngx_shm_zone_t *))
                    != NGX_OK)
            {
                return NGX_CONF_ERROR;
            }

            zones = rlcf->req_zones.elts;
        }

        for (m = 0; m < rlcf->req_zones.nelts; m++) {
            if (shm_zone == zones[m]) {
                return "is duplicate";
            }
        }

        pzone = ngx_array_push(&rlcf->req_zones);
        if (pzone == NULL){
            return NGX_CONF_ERROR;
        }

        *pzone = shm_zone;
    }

    return NGX_CONF_OK;
}
