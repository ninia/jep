
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

InstallDir "$PROGRAMFILES\Jep"

LicenseText "GPL"
LicenseData "..\gpl.txt"

Page license
Page directory
Page instFiles

Section "Install"
SectionIn 2

	SetOutPath $SYSDIR
	File Release\jep.dll
	
	SetOutPath $INSTDIR
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
