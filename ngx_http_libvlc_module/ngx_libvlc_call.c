
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

  char *smem_options = malloc( 1000*sizeof(char*) );
  char *index_location = malloc( 1000*sizeof(char*) ); 
  char *index_url =  malloc( 1000*sizeof(char*) );
	char *ts_location = malloc( 1000*sizeof(char*) );
	char *log_file = malloc( 1000*sizeof(char*) );
	
	char *location = malloc( 1000*sizeof(char*) );


  memset(smem_options, '\0',1000);
memset(index_location, '\0',1000);
memset(index_url, '\0',1000);
memset(ts_location, '\0',1000);
memset(log_file, '\0',1000);
memset(location, '\0',1000);

	struct stat st;
	sprintf(location, "%s/%s",option->index_prefix_location,option->request_id);
	if (stat(location, &st) == -1) {
		    mkdir(location, 0775);
	}

  printf("URI %s \n",uri);

	sprintf(index_location, "%s/%s/list.m3u8", option->index_prefix_location,option->request_id);
	sprintf(index_url, "%s/%s/data-#########.ts", option->index_prefix_url,option->request_id);
	sprintf(ts_location, "%s/%s/data-#########.ts", option->ts_prefix_location,option->request_id);
	sprintf(log_file,"/app/logs/libvlc/test.log",option->request_id);

  // printf("1\n");


    printf("%s \n",index_location);
    printf("%s \n",index_url);
    printf("%s \n",ts_location);
    printf("%s \n",smem_options);
    printf("%s \n",log_file);
    printf("%s \n",uri);

    // printf("2\n");

    const char * const vlc_args[] = {
              "-I", "dummy", // Don't use any interface
              "--ignore-config", // Don't use VLC's config
              "vlc://quit", // 
              "-vvv", // Be verbose,
              "--file-logging", //enable logfile
              "--logfile", log_file, 
              "--sout", smem_options // Stream to memory
               };
    
    

    printf("3\n");      

    inst = libvlc_new( sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
    
    printf("4\n");

    // create a new item
    m = libvlc_media_new_path(inst, uri);

    // printf("5\n");
    // create a media play playing environment
    mp = libvlc_media_player_new_from_media(m);


    // printf("6\n");
    //event manager
    libvlc_event_manager_t* p_em = libvlc_media_player_event_manager( mp );
    libvlc_event_attach(p_em, libvlc_MediaPlayerEndReached, hls_callback, (void *)cdata );


    // printf("7\n");
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