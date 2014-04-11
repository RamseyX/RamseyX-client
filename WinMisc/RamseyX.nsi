;NSIS for RamseyX, ModernUI, Unicode, Version 3.0a2
;Based on NSIS Examples
;ALWAYS USE LZMA AND SOLID!
;Use Vim for these awesome syntax highlights~
  !include "MUI2.nsh"

  Name "RamseyX"
  OutFile "RamseyX_5_0_3.exe"
  InstallDir "$PROGRAMFILES\RamseyX"
  InstallDirRegKey HKCU "Software\RamseyX" ""
  RequestExecutionLevel administrator
  Var StartMenu

  !define MUI_ABORTWARNING

;-----Pages-----
  !insertmacro MUI_PAGE_LICENSE "COPYING" ;Change this....BTW create a link to the GitHub Repo wiki
  ;!insertmacro MUI_PAGE_COMPONENTS (not needed)
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES

  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\RamseyX" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenu

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  ; Well, do we need a data dir choice page?
;---------------
  !insertmacro MUI_LANGUAGE "SimpChinese" ;First is default
  !insertmacro MUI_LANGUAGE "TradChinese"
  !insertmacro MUI_LANGUAGE "English"

; Installer Sections
Section "Dummy Section" SecDummy

  SetOutPath "$INSTDIR"
  
  ;ADD YOUR OWN FILES HERE...
  
  WriteRegStr HKCU "Software\RamseyX" "" $INSTDIR ;Store installation folder
  WriteUninstaller "$INSTDIR\Uninstall.exe" ;Create uninstaller

SectionEnd

; Descriptions

  ;Language strings
  ;USE A LANGUAGE STRING IF YOU WANT YOUR DESCRIPTIONS TO BE LANGAUGE SPECIFIC
  LangString DESC_SecDummy ${LANG_ENGLISH} "Foo_Bar" ;Stub

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDummy} $(DESC_SecDummy) ;See that Varible?
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...

  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenu
  Delete "$SMPROGRAMS\$StartMenu\Uninstall.lnk" ;Please help test something like *.lnk
  RMDir "$SMPROGRAMS\$StartMenu"
  DeleteRegKey /ifempty HKCU "RamseyX"

SectionEnd
Function un.onInit

  !insertmacro MUI_UNGETLANGUAGE
  
FunctionEnd
