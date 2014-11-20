Original Url:
http://forum.xda-developers.com/showthread.php?t=568281

Intro:
This is an all-in-one tool for decompiling, compiling and comparing lua scripts found in Manila (TouchFLO 3D / Sense).
All this is a continuation of sztupy's original work: Lua 5.1 tools. 

General:
LuaTool consists of 4 parts: Lua decompiler, Lua compiler, Lua compare utility and a Manila file type detection utility.

LuaDec 3.2 - Lua decompiler
Notes on latest version:
Major overhaul of the local finding algorithm. Most lua scripts can now be fully decompiled without a problem.
Manila 2.5.1921 has a total of 703 scripts (including embedded scripts). LuaDec can fully decompile 663 files. That's a success rate of 94.31&#37;. 
General notes:
LuaDec automatically checks if the output file was decompiled successfully. 
If it wasn't, LuaDec will also output the disassembly and compare file.
In case the decompile was 100% good, LuaDec will only output the standard .lua file as before.

LuaC 1.2 - Lua compiler
Binary function replacement:
LuaC can directly replace functions in compiled luac files. This can be useful if the luac file can't be fully decompiled, but only a small part of the file needs to be edited. Some more info on function replacement.
Continue statement:
The "continue" statement has been added to the Lua Compiler.
Lua doesn't officially support continue statements, but it looks like HTC added it for their needs, so I'm following their lead.
