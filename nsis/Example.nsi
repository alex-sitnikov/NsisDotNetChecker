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

  ;Option 1: this is the original function
  ;It installs the .NET version if needed, and if that installer requires a reboot it will reboot in the middle of this call.
  ;Doesn't send a return, if the .NET installer needs a restart it will, in the middle of your install.
  ;.NET Installer 4.x will definetly need a reboot if any thing is currently loaded and using any .NET 4.x assemblies.
  !insertmacro CheckNetFramework 472
  
  ;Option 2: If you want to delay a restart with a .NET install use this call, $0 will have true/false as to an install happened.
  ;First Parameter == Same .NET version as used above (version minus the seperations '.')
  ;Second Parameter == returned as "true" if a .NET Install happened, "false" otherwise.
  !insertmacro CheckNetFrameworkDelayRestart 48 $0
  DetailPrint "Did we install a .NET Framework: $0"


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