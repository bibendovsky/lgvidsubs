LgVidSubs

Modified "lgvid" from unofficial NewDark patch
to support external subtitles in movies.

Source code and releases available at
https://github.com/bibendovsky/lgvidsubs

Original link to the NewDark patch:
http://ariane4ever.free.fr/ariane4ever/viewtopic.php?f=2&t=4287&start=0&st=0&sk=t&sd=a&sid=5fb30e2ac9f6a8491c14c973b7176eb1


System requirements
-------------------
NewDark patch (v1.20 Thief/Thief2, v2.42 System Shock 2).


Installation
------------
Extract "lgvid.dll" into the game folder.


Compilation
-----------

You need Visual Studio 2013 or higher (express/full) to compile the project.

Dependencies:
1) Microsoft DirectX SDK (June 2010) headers and libraries.
2) FFMpeg 2.2.1 (or higher) headers and static libraries.


Configuration options for "cam_ext.cfg"
---------------------------------------

Note, some values can be marked as percents. In that case the
value will be converted to pixels by the folowing expression:

value_pixels = screen_height * value_percents / 100


subs_font_filename <filename>
-----------------------------
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

(See LOGFONT structure in MSDN for more info:
 http://msdn.microsoft.com/library/windows/desktop/dd145037%28v=vs.85%29.aspx)


subs_font_color <r> <g> <b> <a>
subs_font_color #rrggbbaa
-------------------------------
Text color.
Default value (fp): 1.0 1.0 1.0 1.0 (solid white)
Default value (#): #ffffffff (solid white)
Valid range of color component (fp): 0.0 .. 1.0
Valid range of color component (#): 00 .. ff


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
subs_font_weight 700
subs_font_color #cc0000ff
subs_shadow_color 0.0 0.0 0.0 0.7
subs_shadow_offset_x 2
subs_shadow_offset_y 1
subs_space_after 4.0%
