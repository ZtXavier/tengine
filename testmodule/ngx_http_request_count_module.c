#include<ngx_http.h>

char *
ngx_http_request_count_set(ngx_conf_t *cf)
{

}


static 
ngx_command_t ngx_http_request_count_cmds[] = {
    {
        ngx_string("count"),
        NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
        ngx_http_request_count_set,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
    ngx_null_command
};

static ngx_http_module_t ngx_http_request_count_ctx{

}



ngx_module_t ngx_http_request_count_module = {
    NGX_MODULE_V1,
    NULL,
    NULL,
    NGX_HTTP_MODULE,

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING
}
