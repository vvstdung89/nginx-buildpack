#!/bin/bash
# Build NGINX and modules on Heroku.
# This program is designed to run in a web dyno provided by Heroku.
# We would like to build an NGINX binary for the builpack on the
# exact machine in which the binary will run.
# Our motivation for running in a web dyno is that we need a way to
# download the binary once it is built so we can vendor it in the buildpack.
#
# Once the dyno has is 'up' you can open your browser and navigate
# this dyno's directory structure to download the nginx binary.

NGINX_VERSION=1.7.5
PCRE_VERSION=8.35
HEADERS_MORE_VERSION=0.23
VLC_VERSION=2.1.5

nginx_tarball_url=http://nginx.org/download/nginx-${NGINX_VERSION}.tar.gz
pcre_tarball_url=http://garr.dl.sourceforge.net/project/pcre/pcre/${PCRE_VERSION}/pcre-${PCRE_VERSION}.tar.bz2
headers_more_nginx_module_url=https://github.com/agentzh/headers-more-nginx-module/archive/v${HEADERS_MORE_VERSION}.tar.gz
vlc_url=http://download.videolan.org/pub/videolan/vlc/${VLC_VERSION}/vlc-${VLC_VERSION}.tar.xz

if (test -f .compile_nginx); then 
	temp_dir=$(mktemp -d /tmp/nginx.XXXXXXXXXX)
	echo "Move module ngx_http_libvlc_module to /${temp_dir}"
	cp -r ngx_http_libvlc_module /${temp_dir}/.

	echo "Serving files from /tmp on $PORT"
	cd /tmp
	python -m SimpleHTTPServer $PORT &

	cd $temp_dir
	echo "Temp dir: $temp_dir"

	echo "Downloading $nginx_tarball_url"
	curl -L $nginx_tarball_url | tar xzv

	echo "Downloading $pcre_tarball_url"
	(cd nginx-${NGINX_VERSION} && curl -L $pcre_tarball_url | tar xvj )

	echo "Downloading $headers_more_nginx_module_url"
	(cd nginx-${NGINX_VERSION} && curl -L $headers_more_nginx_module_url | tar xvz )

	echo "Compile nginx with http_libvlc module"
	(	
		
		cd nginx-${NGINX_VERSION}
		./configure \
			--with-pcre=pcre-${PCRE_VERSION} \
			--prefix=/tmp/nginx \
			--add-module=/${temp_dir}/nginx-${NGINX_VERSION}/headers-more-nginx-module-${HEADERS_MORE_VERSION} \
			--add-module=/${temp_dir}/ngx_http_libvlc_module 
		make install
		
	)
fi

if (test -f .compile_vlc); then 
	#luac rename
	ln -s /app/.apt/usr/bin/luac5.2 /app/.apt/usr/bin/luac
	//ln -s /usr/lib/x86_64-linux-gnu/libgcrypt.so /app/.apt/usr/lib/x86_64-linux-gnu/libgcrypt.so

	# ln -s /usr/lib/x86_64-linux-gnu/libgcrypt.so /app/.apt/usr/lib/.
	# export LIBRARY_PATH=$LIBRARY_PATH:/usr/lib/x86_64-linux-gnu/:

	temp_dir=$(mktemp -d /tmp/vlc.XXXXXXXXXX)
	cur_dir=`pwd`
	cd $temp_dir

	echo "Downloading $vlc_url"
	(curl -L $vlc_url -O && tar xf vlc-${VLC_VERSION}.tar.xz && rm vlc-${VLC_VERSION}.tar.xz)

	echo "Compile vlc"
	(	
		cd $cur_dir	
		if (test -d vlc-${VLC_VERSION}); then
			echo "Copy file to /${temp_dir}/vlc-${VLC_VERSION}"
			cp -rf vlc-${VLC_VERSION}/* /${temp_dir}/vlc-${VLC_VERSION}/. 
		fi
		cd /${temp_dir}/vlc-${VLC_VERSION}
		# ./bootstrap
		./configure \
			--disable-glx \
			--enable-libgcrypt \
			--enable-dvbpsi \
			--enable-avcodec \
			--enable-avformat  \
			--enable-x264 \
			--disable-mad \
			--disable-swscale \
			--disable-a52 \
			--prefix=/tmp/vlc 
		make install

		cd /tmp
		tar -zcvf vlc.tar.gz /tmp/vlc
	)	
fi


while true
do
	sleep 1
	echo "."
done
