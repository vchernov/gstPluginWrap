#!/bin/sh

PKG_CONFIG_PATH=$1
: ${PKG_CONFIG_PATH:=/usr/local/lib/pkgconfig}

INSTALL_PATH=$2
: ${INSTALL_PATH:=$PWD}

TEMPLATE_FILE=gstPluginWrap.pc.template
OUT_FILE=${PKG_CONFIG_PATH}/gstPluginWrap.pc
VERSION=`head -1 VERSION`

if echo "$(eval "echo \"$(cat $TEMPLATE_FILE)\"")" > $OUT_FILE; then
	echo "$OUT_FILE has been created"
	echo "now you can run \"pkg-config --cflags gstPluginWrap\" and \"pkg-config --libs gstPluginWrap\""
fi
