#!/bin/bash

#
# Extra minimum configuration.
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
    --disable-decoders \
    --enable-decoder=pcm_s8 \
    --enable-decoder=pcm_u8 \
    --enable-decoder=pcm_s16be \
    --enable-decoder=pcm_s16le \
    --enable-decoder=pcm_u16be \
    --enable-decoder=pcm_u16le \
    --enable-decoder=indeo5 \
    --enable-decoder=mp3 \
    --enable-decoder=vp3 \
    --disable-muxers \
    --disable-demuxers \
    --enable-demuxer=avi \
    --disable-parsers \
    --enable-parser=vp3 \
    --disable-bsfs \
    --disable-protocols \
    --disable-devices \
    --disable-filters \
    --enable-memalign-hack \
    --disable-debug \
