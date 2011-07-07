!include "MUI2.nsh"
!include "DotNetChecker.nsh"

Name "DotNetChecker Example"
OutFile "Example.exe"

InstallDir "$LOCALAPPDATA\DotNetChecker Example" 
InstallDirRegKey HKCU "Software\DotNetChecker Example" ""

RequestExecutionLevel admin

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
 
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
  
!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "Dummy Section" SecDummy

  SetOutPath "$INSTDIR"
  
  !insertmacro CheckNetFramework 40Client
  
  ;Store installation folder
  WriteRegStr HKCU "Software\DotNetChecker Example" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

Section "Uninstall"

  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"
  DeleteRegKey /ifempty HKCU "Software\DotNetChecker Example"
SectionEnd