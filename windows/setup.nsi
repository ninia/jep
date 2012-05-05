
; nsis installer for jep

!ifdef NOCOMPRESS
SetCompress off
!endif

SetDateSave on
SetDatablockOptimize on
CRCCheck on
; SilentInstall normal
; BGGradient 000000 800000 FFFFFF
; InstallColors FF8080 000030

Name "Java Embedded Python"
OutFile "setup.exe"

InstallDir "$PROGRAMFILES\Jepp"

LicenseText "zlib/libpng"
LicenseData "..\LICENSE"

Page license
Page directory
Page instFiles

Section "Install"
SectionIn 2

	SetOutPath $SYSDIR
	File Active\jep.dll
	
	SetOutPath $INSTDIR
    File c:\windows\system32\MSVCR71.DLL
    File c:\windows\system32\MSVCRT.DLL
	File ..\jep.jar
	File ..\README
	File ..\console.py
	WriteUninstaller $INSTDIR\uninstall.exe
        
SectionEnd

; special uninstall section.
Section "Uninstall"

	Delete "$INSTDIR\*.*"
	RMDir "$INSTDIR"
	Delete "$SYSDIR\jep.dll"

SectionEnd
