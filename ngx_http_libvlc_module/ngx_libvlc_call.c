
#include "ngx_libvlc_call.h"
#include "ngx_libvlc_utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>

pthread_t leak_memory_collect;

void* doGarbageCollect(void *arg)
{
	callback_media_data *data = (callback_media_data *) arg;
    sleep(3);
    printf("garbage thread\n");
    

	// stop playing
    libvlc_media_player_stop((libvlc_media_player_t *) *data->mp);

	//free the media_player
	libvlc_media_player_release((libvlc_media_player_t *) *data->mp);

	libvlc_release((libvlc_instance_t *) *data->inst);

  free(data);
    return NULL;
}


void hls_callback(const libvlc_event_t* event, void* ptr){
	callback_media_data *data = (callback_media_data *) ptr;
	switch ( event->type )	{
		case libvlc_MediaPlayerEndReached:

			printf("call back call -> endreach\n");
			int err = pthread_create(&(leak_memory_collect), NULL, &doGarbageCollect, data);
	        if (err != 0)
	            printf("\ncan't create thread :[%s]", strerror(err));
	        else
	            printf("\n Thread created successfully\n");
        break;

	}
}


int ngx_libvlc_hls_convert(char* uri, hls_transcode_option* option, char* res) {

	libvlc_instance_t *inst;
    libvlc_media_player_t *mp;
    libvlc_media_t *m;
	callback_media_data *cdata = (callback_media_data*) malloc( sizeof(callback_media_data*) );

    char smem_options[1000];
  	char index_location[1000];
  	char index_url[1000];
	char ts_location[1000];
	char log_file[100];
	
	char location[100];
	struct stat st;
	sprintf(location, "%s/%s",option->index_prefix_location,option->request_id);
	if (stat(location, &st) == -1) {
		    mkdir(location, 0775);
	}

	sprintf(index_location, "%s/%s/list.m3u8", option->index_prefix_location,option->request_id);
	sprintf(index_url, "%s/%s/data-#########.ts", option->index_prefix_url,option->request_id);
	sprintf(ts_location, "%s/%s/data-#########.ts", option->ts_prefix_location,option->request_id);
	sprintf(log_file,"/var/log/ngx-libvlc/%s.log",option->request_id);

    sprintf(smem_options
      , "#transcode { vcodec=h264,vb=512,scale=1,acodec=none,venc=x264{aud,profile=baseline,level=30,keyint=15, bframes=0,ref=1,nocabac}}:duplicate{dst=std{access=livehttp{"
         "seglen=10,  numsegs=5,delsegs=true,index=%s,"
         "index-url=%s},dst=%s,mux=ts{use-key-frames} }"
      , index_location //index file location-> access through web service ex: /var/www/html/hls/f8ee546866a511e4a73f22000b0f0a0c/list.m3u8
      , index_url //http://127.0.0.1/f8ee546866a511e4a73f22000b0f0a0c/data-#########.ts
      , ts_location  //ts file location-> access through web service ex: /var/www/html/hls/f8ee546866a511e4a73f22000b0f0a0c/data-#########.ts
      );

    // printf("%s \n",index_location);
    // printf("%s \n",index_url);
    // printf("%s \n",ts_location);
    // printf("%s \n",smem_options);
    // printf("%s \n",log_file);
    // printf("%s \n",uri);


    const char * const vlc_args[] = {
              "-I", "dummy", // Don't use any interface
              "--ignore-config", // Don't use VLC's config
              "vlc://quit", // 
              "-v", // Be verbose,
              "--file-logging", //enable logfile
              "--logfile", log_file, 
              "--sout", smem_options // Stream to memory
               };


    inst = libvlc_new( sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
    
    printf("URI %s \n",uri);

    // create a new item
    m = libvlc_media_new_path(inst, uri);

    // create a media play playing environment
    mp = libvlc_media_player_new_from_media(m);

    //event manager
    libvlc_event_manager_t* p_em = libvlc_media_player_event_manager( mp );
    libvlc_event_attach(p_em, libvlc_MediaPlayerEndReached, hls_callback, (void *)cdata );

    // no need to keep the media now
    libvlc_media_release(m);

    // play the media_player
    libvlc_media_player_play(mp);


   


    printf("%s \n","play media");
    // wait for file present
    int counter = 0;
    while (counter++ <= 150) { // Time out set to 60 seconds
    	sleep(1);
    	//Check file present
		if (doesFileExist(index_location)){
			printf("%s \n","file exit; return ");
			cdata->mp=&mp;
			cdata->m=&m;
			cdata->inst=&inst;
			sprintf(res,"%s/%s/list.m3u8",option->index_prefix_url,option->request_id);
			return 0;
		}
    }

    // stop playing
    libvlc_media_player_stop(mp);

    // free the media_player
    libvlc_media_player_release(mp);

    libvlc_release(inst);
    sprintf(res,"Request time out");

	return -1;
}