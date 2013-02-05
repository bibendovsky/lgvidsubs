#!/bin/bash

#
# Common stuff.
#

if [ $# -eq 0 ]; then
    echo "Please specify FFmpeg version."
    exit 1
fi

FF_VERSION=$1
FF_DIR=$(readlink -efn ../ffmpeg-${FF_VERSION})

if [ ! -d $FF_DIR ]; then
    echo "Directory \"$FF_DIR\" not found."
    exit 2
fi
