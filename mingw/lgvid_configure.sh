#!/bin/bash

#
# Maximum (default) configuration.
#

source lgvid_common.sh

${FF_DIR}/configure \
    --enable-gpl \
    --enable-version3 \
    --enable-runtime-cpudetect \
    --disable-doc \
    --disable-htmlpages \
    --disable-manpages \
    --disable-podpages \
    --disable-txtpages \
    --disable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
    --disable-ffserver \
    --disable-avdevice \
    --disable-postproc \
    --disable-avfilter \
    --disable-network \
    --disable-encoders \
    --disable-muxers \
    --disable-protocols \
    --disable-devices \
    --disable-filters \
    --enable-memalign-hack \
    --disable-debug \
