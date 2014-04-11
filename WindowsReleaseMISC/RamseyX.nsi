;NSIS for RamseyX, ModernUI, Unicode
;Version 3.0a2
;Based on NSIS Examples
;ALWAYS USE LZMA AND SOLID!
 !include "MUI2.nsh"

  Name "RamseyX"
  OutFile "RamseyX_5_0_3.exe"
  InstallDir "$PROGRAMFILES\RamseyX"
  InstallDirRegKey HKCU "Software\RamseyX" ""
  RequestExecutionLevel administrator

  !define MUI_ABORTWARNING

;-----Pages-----
  !insertmacro MUI_PAGE_LICENSE "COPYING" ;Change this....
  ;!insertmacro MUI_PAGE_COMPONENTS (not needed)
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
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
  
  ;Store installation folder
  WriteRegStr HKCU "Software\RamseyX" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

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
  DeleteRegKey /ifempty HKCU "RamseyX"

SectionEnd
Function un.onInit

  !insertmacro MUI_UNGETLANGUAGE
  
FunctionEnd
