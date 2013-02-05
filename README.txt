LgVidSubs

Modified "lgvid" from unofficial DarkEngine patch
to support external subtitles in movies.

Boris I. Bendovsky
---------------------------------------------------

Original link to unofficial patch:
http://ariane4ever.free.fr/ariane4ever/viewtopic.php?f=2&t=4287&start=0&st=0&sk=t&sd=a&sid=5fb30e2ac9f6a8491c14c973b7176eb1


System requirements
-------------------
Same as unofficial DarkEngine patch.


Installation
------------
Extract "lgvid.dll" and "d3d9.dll" into the game folder.


Configuration options for "cam_ext.cfg"
---------------------------------------

Note, some values can be marked as percents. In that case the
value will be converted to pixels by the folowing expression:

value_pixels = screen_height * value_percents / 100


subs_font_filename <filename>
-----------------------
Temporarily registers specified external font.
Default value: <empty>


subs_font_family <name>
-----------------------
Font family name.
Default value: Arial


subs_font_size <pixels|percents%>
---------------------------------
Font size.
Default value: 7%
Valid range (%):  1.5 .. 13
Valid range (pixels): 8 .. 64


subs_font_weight <value>
------------------------
Font size.
Default value: 0
Valid range:  0 .. 1000

Some useful values:
0 - don't care/default;
300 - light;
400 - normal;
700 - bold.

(See LOGFONT structure in MSDN for more info.)



subs_font_color <r> <g> <b> <a>
-------------------------------
Text color.
Default value: 1.0 1.0 1.0 1.0 (solid white)
Valid range of component: 0.0 .. 1.0


subs_shadow_color <r> <g> <b> <a>
---------------------------------
Text's shadow color.
Default value: 0.0 0.0 0.0 0.0 (solid black)
Valid range of component: 0.0 .. 1.0


subs_shadow_offset_x <pixels|percents%>
---------------------------------------
Text's shadow offset by X.
Default value: 0.42%
Valid range (%):  -1.45 .. 1.45
Valid range (pixels): -7 .. 7


subs_shadow_offset_y <pixels|percents%>
---------------------------------------
Text's shadow offset by Y.
Default value: 0.42%
Valid range (%):  -1.45 .. 1.45
Valid range (pixels): -7 .. 7


subs_space_after <pixels|percents%>
-----------------------------------
Offset of text of screen's bottom.
Default value: 3.5%
Valid range (%):  0 .. 13
Valid range (pixels): 0 .. 64


Example:
--------
subs_font_filename my_arial.ttf
subs_font_family arial
subs_font_size 8.0%
subs_font_color 0.8 1.0 0.8 1.0
subs_shadow_color 0.0 0.0 0.0 0.7
subs_shadow_offset_x 2
subs_shadow_offset_y 1
subs_space_after 4.0%



==========
Change log
----------

----------
2013.02.05
----------

Added new configuration option - "subs_font_filename".
Moved back to MSVC 2010 EE.
Removed MinGW workarounds for string conversion (char -> wchar_t).
Updated FFmpeg to version 1.1.1.


----------
2013.01.11
----------

Integrated FFmpeg (v1.1) into the lgvid.dll.
Minor code changes for compiling lgvid.dll with MinGW.


----------
2012.11.15
----------

Moved on to MSVC 2010 EE.
Replaced a GDI rendering with a Direct3D 9 (via wrapper dll) one.
Added new configuration options to "cam_ext.cfg".


----------
2012.11.09
----------

Added very basic support for .srt subtitles:
  - text in OEM encoding;
  - blank line tag (\n).

Subtitles renders on original frame picture.
No (yet) options for font parameters (family, size, etc.).
 