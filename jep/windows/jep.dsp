# Microsoft Developer Studio Project File - Name="jep" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=JEP - WIN32 RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "jep.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "jep.mak" CFG="JEP - WIN32 RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "jep - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JEP_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W2 /WX /Gi /GX /O2 /I "C:\j2sdk1.4.2_04\include" /I "C:\j2sdk1.4.2_04\include\win32" /I "C:\Python23\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JEP_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /I "C:\j2sdk1.4.2_04\include" /I "C:\j2sdk1.4.2_04\include\win32" /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "C:\j2sdk1.4.2_04\include" /i "C:\j2sdk1.4.2_04\include\win32" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /libpath:"C:\Python23\libs"
# Begin Target

# Name "jep - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\jep.c
# End Source File
# Begin Source File

SOURCE=..\pyembed.c
# End Source File
# Begin Source File

SOURCE=..\pyjarray.c
# End Source File
# Begin Source File

SOURCE=..\pyjclass.c
# End Source File
# Begin Source File

SOURCE=..\pyjfield.c
# End Source File
# Begin Source File

SOURCE=..\pyjmethod.c
# End Source File
# Begin Source File

SOURCE=..\pyjobject.c
# End Source File
# Begin Source File

SOURCE=..\util.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\jep.h
# End Source File
# Begin Source File

SOURCE=..\pyembed.h
# End Source File
# Begin Source File

SOURCE=..\pyjarray.h
# End Source File
# Begin Source File

SOURCE=..\pyjclass.h
# End Source File
# Begin Source File

SOURCE=..\pyjfield.h
# End Source File
# Begin Source File

SOURCE=..\pyjmethod.h
# End Source File
# Begin Source File

SOURCE=..\pyjobject.h
# End Source File
# Begin Source File

SOURCE=..\system.h
# End Source File
# Begin Source File

SOURCE=..\util.h
# End Source File
# Begin Source File

SOURCE=.\winconfig.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
