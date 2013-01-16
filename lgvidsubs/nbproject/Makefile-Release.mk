#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc.exe
CCC=g++.exe
CXX=g++.exe
FC=gfortran
AS=as.exe

# Macros
CND_PLATFORM=MinGW-Windows
CND_DLIB_EXT=dll
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/1200762390/bbi_dll_context.o \
	${OBJECTDIR}/_ext/1200762390/bbi_subtitle.o \
	${OBJECTDIR}/_ext/1200762390/bbi_string_helper.o \
	${OBJECTDIR}/_ext/1200762390/bbi_srt_parser.o \
	${OBJECTDIR}/src/lgvid.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-m32
CXXFLAGS=-m32

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../../nde-ffmpeg/libavcodec/libavcodec.a ../../nde-ffmpeg/libavformat/libavformat.a ../../nde-ffmpeg/libavutil/libavutil.a ../../nde-ffmpeg/libswresample/libswresample.a ../../nde-ffmpeg/libswscale/libswscale.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lgvid.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lgvid.${CND_DLIB_EXT}: ../../nde-ffmpeg/libavcodec/libavcodec.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lgvid.${CND_DLIB_EXT}: ../../nde-ffmpeg/libavformat/libavformat.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lgvid.${CND_DLIB_EXT}: ../../nde-ffmpeg/libavutil/libavutil.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lgvid.${CND_DLIB_EXT}: ../../nde-ffmpeg/libswresample/libswresample.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lgvid.${CND_DLIB_EXT}: ../../nde-ffmpeg/libswscale/libswscale.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lgvid.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -static -shared -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lgvid.${CND_DLIB_EXT} -s ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/1200762390/bbi_dll_context.o: ../lgvidsubs_shared/bbi_dll_context.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1200762390
	${RM} $@.d
	$(COMPILE.cc) -O2 -s -Isrc -Isrc/ffmpeg -I../lgvidsubs_shared -I../../ffmpeg-1.1 -std=c++98  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1200762390/bbi_dll_context.o ../lgvidsubs_shared/bbi_dll_context.cpp

${OBJECTDIR}/_ext/1200762390/bbi_subtitle.o: ../lgvidsubs_shared/bbi_subtitle.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1200762390
	${RM} $@.d
	$(COMPILE.cc) -O2 -s -Isrc -Isrc/ffmpeg -I../lgvidsubs_shared -I../../ffmpeg-1.1 -std=c++98  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1200762390/bbi_subtitle.o ../lgvidsubs_shared/bbi_subtitle.cpp

${OBJECTDIR}/_ext/1200762390/bbi_string_helper.o: ../lgvidsubs_shared/bbi_string_helper.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1200762390
	${RM} $@.d
	$(COMPILE.cc) -O2 -s -Isrc -Isrc/ffmpeg -I../lgvidsubs_shared -I../../ffmpeg-1.1 -std=c++98  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1200762390/bbi_string_helper.o ../lgvidsubs_shared/bbi_string_helper.cpp

${OBJECTDIR}/_ext/1200762390/bbi_srt_parser.o: ../lgvidsubs_shared/bbi_srt_parser.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1200762390
	${RM} $@.d
	$(COMPILE.cc) -O2 -s -Isrc -Isrc/ffmpeg -I../lgvidsubs_shared -I../../ffmpeg-1.1 -std=c++98  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1200762390/bbi_srt_parser.o ../lgvidsubs_shared/bbi_srt_parser.cpp

${OBJECTDIR}/src/lgvid.o: src/lgvid.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -s -Isrc -Isrc/ffmpeg -I../lgvidsubs_shared -I../../ffmpeg-1.1 -std=c++98  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/lgvid.o src/lgvid.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lgvid.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
