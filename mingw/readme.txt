Compiling FFmpeg with MinGW.


Expected directory structure:

...
+- ffmpeg-x.y.z
+- ffmpeg-x.y.z-lgvid
+- lgvidsubs
       +- lgvidsubs
       +- lgvidsubs_d3d
       +- lgvidsubs_shared
      ...
...

where x.y.z - real version of FFmpeg.


Extract FFmpeg sources into "ffmpeg-x.y.z".

Copy "lgvid_common.sh", "lgvid_configure.sh" (all codecs, etc.) or
"lgvid_configure_min.sh" (minimum codecs, etc.) into "ffmpeg-x.y.z-lgvid".
Run in Msys "lgvid_configure.sh"/"lgvid_configure_min.sh" passing FFmpeg version
(i.e. "lgvid_configure.sh" 1.2.3). After successfull configuring run make.

P.S. Do not forget to change FFmpeg version (and possibly GCC's too) in
Visual Studio project "lgvidsubs".
