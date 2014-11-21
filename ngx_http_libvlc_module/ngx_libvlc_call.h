#ifndef NGX_LIBVLC_CALL_H
#define NGX_LIBVLC_CALL_H

#include <vlc/vlc.h>

typedef struct {
	char* request_id;
	char* index_prefix_location;
	char* index_prefix_url;
	char* ts_prefix_location;
} hls_transcode_option;

typedef struct {
   libvlc_media_player_t **mp;
   libvlc_media_t **m;
   libvlc_instance_t **inst;
} callback_media_data;

extern int ngx_libvlc_hls_convert(char* uri, hls_transcode_option* option , char* res);

#endif 