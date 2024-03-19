cl.exe /c /EHsc OGL.cpp
rc.exe OGL.rc
link.exe OGL.obj OGL.res User32.lib GDI32.lib /SUBSYSTEM:WINDOWS
OGL.exe
pause