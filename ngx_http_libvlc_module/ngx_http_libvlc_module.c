#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "string.h"
#include "ngx_libvlc_call.h"
#include "ngx_libvlc_utils.h"

static char *ngx_http_libvlc(ngx_conf_t *cf, void *post, void *data);

static ngx_conf_post_handler_pt ngx_http_libvlc_p = ngx_http_libvlc;

/*
 * The structure will holds the value of the 
 * module directive libvlc
 */
typedef struct {
    ngx_str_t   name;
} ngx_http_libvlc_loc_conf_t;

/* The function which initializes memory for the module configuration structure       
 */
static void *
ngx_http_libvlc_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_libvlc_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_libvlc_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }
    
    return conf;
}

/* 
 * The command array or array, which holds one subarray for each module 
 * directive along with a function which validates the value of the 
 * directive and also initializes the main handler of this module
 */
static ngx_command_t ngx_http_libvlc_commands[] = {
    { 
      ngx_string("transcode"),
      NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_libvlc_loc_conf_t, name),
      &ngx_http_libvlc_p },
 
    ngx_null_command
};
 
 
static ngx_str_t hello_string;
 
/*
 * The module context has hooks , here we have a hook for creating
 * location configuration
 */
static ngx_http_module_t ngx_http_libvlc_module_ctx = {
    NULL,                          /* preconfiguration */
    NULL,                          /* postconfiguration */
 
    NULL,                          /* create main configuration */
    NULL,                          /* init main configuration */
 
    NULL,                          /* create server configuration */
    NULL,                          /* merge server configuration */
 
    ngx_http_libvlc_create_loc_conf, /* create location configuration */
    NULL                           /* merge location configuration */
};
 

/*
 * The module which binds the context and commands 
 * 
 */
ngx_module_t ngx_http_libvlc_module = {
    NGX_MODULE_V1,
    &ngx_http_libvlc_module_ctx,    /* module context */
    ngx_http_libvlc_commands,       /* module directives */
    NGX_HTTP_MODULE,               /* module type */
    NULL,                          /* init master */
    NULL,                          /* init module */
    NULL,                          /* init process */
    NULL,                          /* init thread */
    NULL,                          /* exit thread */
    NULL,                          /* exit process */
    NULL,                          /* exit master */
    NGX_MODULE_V1_PADDING
};
 
/*
 * Main handler function of the module. 
 */
static ngx_int_t
ngx_http_libvlc_handler(ngx_http_request_t *r)
{
    ngx_int_t    rc;
    ngx_buf_t   *b;
    ngx_chain_t  out;
 	printf("libvlc handler\n");
 	

    /* we response to 'GET' and 'HEAD' requests only */
    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }
 
    /* discard request body, since we don't need it here */
    rc = ngx_http_discard_request_body(r);
 
    if (rc != NGX_OK) {
        return rc;
    }
 
    /* set the 'Content-type' header */
    r->headers_out.content_type_len = sizeof("text/html") - 1;
    r->headers_out.content_type.len = sizeof("text/html") - 1;
    r->headers_out.content_type.data = (u_char *) "text/html";

    /* send the header only, if the request type is http 'HEAD' */
    if (r->method == NGX_HTTP_HEAD) {
        r->headers_out.status = NGX_HTTP_OK;
        r->headers_out.content_length_n = hello_string.len;
 
        return ngx_http_send_header(r);
    }
 
    /* allocate a buffer for your response body */
    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    




    //checking transcode service again (not neccessary)
    //nginx configuration be done this check instead
    char *prefix_url = malloc(100*sizeof(char*));
    memset(prefix_url, '\0',100);
    char *uri = malloc(500*sizeof(char*));
    memset(uri, '\0',500);
    char* trancode_link_respond = malloc(100*sizeof(char*));
    memset(trancode_link_respond, '\0',100);


    hls_transcode_option*   transcode_option  = (hls_transcode_option*) malloc(sizeof(hls_transcode_option));
   
    

    strcpy(prefix_url,"/transcode/hls/http:/");
    int result = strncmp((char* )r->uri.data, prefix_url, 21);

    


    transcode_option->request_id = (char *)malloc(100*sizeof(char *));// = "id1234556789";
	transcode_option->index_prefix_location = (char*) malloc(100*sizeof(char*));
	transcode_option->index_prefix_url = (char*) malloc(100*sizeof(char*));
	transcode_option->ts_prefix_location = (char*) malloc(100*sizeof(char*));

    memset(transcode_option->request_id, '\0',100);
    memset(transcode_option->index_prefix_location, '\0',100);
    memset(transcode_option->index_prefix_url, '\0', 100);
    memset(transcode_option->ts_prefix_location, '\0', 100);

	generateGUID(transcode_option->request_id);

	// printf("%s \n",transcode_option->request_id);
	run_getenv("INDEX_PREFIX_LOCATION", transcode_option->index_prefix_location);
	run_getenv("INDEX_PREFIX_URL", transcode_option->index_prefix_url);
	run_getenv("TS_PREFIX_LOCATION", transcode_option->ts_prefix_location);
	

    if (result == 0){ //trancode request match match
    	strcpy(uri,"http://");
    	memcpy(uri+7, r->uri.data+21,r->uri.len-21);
    	// sleep(10);
    	//start to call transcode Function
    	
    	if (ngx_libvlc_hls_convert(uri, transcode_option, trancode_link_respond) == 0){
    		// hello_string.data = (u_char *)trancode_link_respond;
    		// hello_string.len = strlen(trancode_link_respond);
    	} else {
    		// hello_string.data = (u_char *)trancode_link_respond;
    		// hello_string.len = strlen(trancode_link_respond);
    	}


    } else {
    	strcpy(trancode_link_respond,"no thing");
        // hello_string.data = (u_char *)trancode_link_respond;
        // hello_string.len = strlen(trancode_link_respond);
        printf("no thing to response\n");
    	//response no thing
    }
    
    /* attach this buffer to the buffer chain */
    out.buf = b;
    out.next = NULL;
 
    /* adjust the pointers of the buffer */
    //r->headers_in
    b->pos = (u_char *)trancode_link_respond;
    b->last = (u_char*)trancode_link_respond + strlen(trancode_link_respond);
    b->memory = 1;    /* this buffer is in memory */
    b->last_buf = 1;  /* this is the last buffer in the buffer chain */
 
    /* set the status line */
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = strlen(trancode_link_respond);
 
    /* send the headers of your response */
    rc = ngx_http_send_header(r);
 
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }
    

    free(prefix_url);
    free(uri);
    free(trancode_link_respond);
    free(transcode_option->request_id);
    free(transcode_option->index_prefix_location);
    free(transcode_option->index_prefix_url);
    free(transcode_option->ts_prefix_location);


    /* send the buffer chain of your response */
    return ngx_http_output_filter(r, &out);
}
 
/*
 * Function for the directive trasncode , it validates its value
 * and copies it to a static variable to be printed later
 */
static char *
ngx_http_libvlc(ngx_conf_t *cf, void *post, void *data)
{
    ngx_http_core_loc_conf_t *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_libvlc_handler;

    // ngx_str_t  *name = data; // i.e., first field of ngx_http_libvlc_loc_conf_t
    
    // if (ngx_strcmp(name->data, "") == 0) {
    //     return NGX_CONF_ERROR;
    // }
    // printf("%s",name->data);
    // hello_string.data = name->data;
    // hello_string.len = ngx_strlen(hello_string.data);

    return NGX_CONF_OK;
}