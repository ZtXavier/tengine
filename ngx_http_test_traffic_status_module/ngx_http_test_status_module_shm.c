#include"ngx_http_test_status_module.h"
#include "ngx_http_test_status_module_shm.h"
#include "ngx_http_test_status_module_node.h"

void
ngx_http_test_traffic_status_shm_info_node(ngx_http_request_t *r,
    ngx_http_test_traffic_status_shm_info_t *shm_info,
    ngx_rbtree_node_t *node)
{
/*     ngx_str_t                              filter; */
    ngx_uint_t                             size;
    ngx_http_test_traffic_status_ctx_t   *ctx;
    ngx_http_test_traffic_status_node_t  *vtsn;

    ctx = ngx_http_get_module_main_conf(r, ngx_http_test_traffic_status_module);

    if (node != ctx->rbtree->sentinel) {
        vtsn = (ngx_http_test_traffic_status_node_t *) &node->color;

        size = offsetof(ngx_rbtree_node_t, color)
               + offsetof(ngx_http_test_traffic_status_node_t, data)
               + vtsn->len;

        shm_info->used_size += size;
        shm_info->used_node++;

       /*  if (vtsn->stat_upstream.type == NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_FG) {
            filter.data = vtsn->data;
            filter.len = vtsn->len;

            (void) ngx_http_test_traffic_status_node_position_key(&filter, 1);

            if (ngx_http_test_traffic_status_filter_max_node_match(r, &filter) == NGX_OK) {
                shm_info->filter_used_size += size;
                shm_info->filter_used_node++;
            }
        } */

        ngx_http_test_traffic_status_shm_info_node(r, shm_info, node->left);
        ngx_http_test_traffic_status_shm_info_node(r, shm_info, node->right);
    }
}


void
ngx_http_test_traffic_status_shm_info(ngx_http_request_t *r,
    ngx_http_test_traffic_status_shm_info_t *shm_info)
{
    ngx_http_test_traffic_status_ctx_t  *ctx;

    ctx = ngx_http_get_module_main_conf(r, ngx_http_test_traffic_status_module);

    ngx_memzero(shm_info, sizeof(ngx_http_test_traffic_status_shm_info_t));

    shm_info->name = &ctx->shm_name;
    shm_info->max_size = ctx->shm_size;

    ngx_http_test_traffic_status_shm_info_node(r, shm_info, ctx->rbtree->root);
}


static ngx_int_t
ngx_http_test_traffic_status_shm_add_node(ngx_http_request_t *r,
    ngx_str_t *key, unsigned type)
{
    size_t                                     size;
/*     unsigned                                   init; */
    uint32_t                                   hash;
    ngx_slab_pool_t                           *shpool;
    ngx_rbtree_node_t                         *node, *lrun;
    ngx_http_test_traffic_status_ctx_t       *ctx;
    ngx_http_test_traffic_status_node_t      *vtsn;
    ngx_http_test_traffic_status_loc_conf_t  *vtscf;
    ngx_http_test_traffic_status_shm_info_t  *shm_info;

    ctx = ngx_http_get_module_main_conf(r, ngx_http_test_traffic_status_module);

    vtscf = ngx_http_get_module_loc_conf(r, ngx_http_test_traffic_status_module);

    if (key->len == 0) {
        return NGX_ERROR;
    }

    shpool = (ngx_slab_pool_t *) vtscf->shm_zone->shm.addr;

    ngx_shmtx_lock(&shpool->mutex);

    /* find node */
    hash = ngx_crc32_short(key->data, key->len);

    node = ngx_http_test_traffic_status_find_node(r, key, type, hash);

    /* set common */
    if (node == NULL) {
       /*  init = NGX_HTTP_TEST_TRAFFIC_STATUS_NODE_NONE; */

        /* delete lru node */
        lrun = ngx_http_test_traffic_status_find_lru(r);
        if (lrun != NULL) {
            ngx_rbtree_delete(ctx->rbtree, lrun);
            ngx_slab_free_locked(shpool, lrun);
        }

        size = offsetof(ngx_rbtree_node_t, color)
               + offsetof(ngx_http_test_traffic_status_node_t, data)
               + key->len;

        node = ngx_slab_alloc_locked(shpool, size);
        if (node == NULL) {
            shm_info = ngx_pcalloc(r->pool, sizeof(ngx_http_test_traffic_status_shm_info_t));
            if (shm_info == NULL) {
                return NGX_ERROR;
            }

            ngx_http_test_traffic_status_shm_info(r, shm_info);

            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                          "shm_add_node::ngx_slab_alloc_locked() failed: "
                          "used_size[%ui], used_node[%ui]",
                          shm_info->used_size, shm_info->used_node);

            ngx_shmtx_unlock(&shpool->mutex);
            return NGX_ERROR;
        }

        vtsn = (ngx_http_test_traffic_status_node_t *) &node->color;

        node->key = hash;
        vtsn->len = (u_short) key->len;
        ngx_http_test_traffic_status_node_init(r, vtsn);
        vtsn->stat_upstream.type = type;
        ngx_memcpy(vtsn->data, key->data, key->len);

        ngx_rbtree_insert(ctx->rbtree, node);

    } else {
        /* init = NGX_HTTP_TEST_TRAFFIC_STATUS_NODE_FIND; */
        vtsn = (ngx_http_test_traffic_status_node_t *) &node->color;
        ngx_http_test_traffic_status_node_set(r, vtsn);
    }

    /* set addition */
//     switch(type) {
//     case NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_NO:
//         break;

//     case NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_UA:
//     case NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_UG:
//         (void) ngx_http_test_traffic_status_shm_add_node_upstream(r, vtsn, init);
//         break;

// #if (NGX_HTTP_CACHE)
//     case NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_CC:
//         (void) ngx_http_test_traffic_status_shm_add_node_cache(r, vtsn, init);
//         break;
// #endif

//     case NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_FG:
//         break;
//     }

/*     vtscf->node_caches[type] = node; */

    ngx_shmtx_unlock(&shpool->mutex);

    return NGX_OK;
}


ngx_int_t
ngx_http_test_traffic_status_shm_add_server(ngx_http_request_t *r)
{
    unsigned                                   type;
    ngx_int_t                                  rc;
    ngx_str_t                                  key, dst;
    ngx_http_core_srv_conf_t                  *cscf;
    ngx_http_test_traffic_status_loc_conf_t  *vtscf;

    vtscf = ngx_http_get_module_loc_conf(r, ngx_http_test_traffic_status_module);

    cscf = ngx_http_get_module_srv_conf(r, ngx_http_core_module);

    if (vtscf->filter && vtscf->filter_host && r->headers_in.server.len) {
        /* set the key by host header */
        dst = r->headers_in.server;

    } else {
        /* set the key by server_name variable */
        dst = cscf->server_name;
        if (dst.len == 0) {
            dst.len = 1;
            dst.data = (u_char *) "_";
        }
    }

    type = NGX_HTTP_TEST_TRAFFIC_STATUS_UPSTREAM_NO;

    rc = ngx_http_test_traffic_status_node_generate_key(r->pool, &key, &dst, type);
    if (rc != NGX_OK) {
        return NGX_ERROR;
    }

    return ngx_http_test_traffic_status_shm_add_node(r, &key, type);
}