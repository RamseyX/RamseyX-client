;NSIS for RamseyX, ModernUI, Unicode, Version 3.0a2
;Based on NSIS Examples
;Use Vim for these awesome syntax highlights~

!include                        "MUI2.nsh"
!include                        "x64.nsh"
!include	                "Sections.nsh"
!define	PRODUCT_NAME            "RamseyX 运算客户端"
!define PRODUCT_VERSION         "4.4.1.0"
!define PRODUCT_PUBLISHER       "RamseyX 小组"
!define	PRODUCT_WEB_SITE        "http://www.ramseyx.org/index.html"
!define MUI_ICON                "RamseyX.ico"
Name                            "RamseyX"
OutFile                         "RamseyX_5_0_3.exe"
InstallDir                      "$APPDATA\RamseyX" ;Permission issues with UAC in $PROGRAMFILES
InstallDirRegKey                HKCU "Software\RamseyX" ""
ShowInstDetails	                show
ShowUninstDetails	        show
SetCompressor /SOLID	        lzma
SetCompressorDictSize	        128
SetDatablockOptimize	        on
BrandingText	                "Copyright(R)2013 RamseyX 小组，保留所有权利"
RequestExecutionLevel           admin

Var StartMenu
  !define MUI_ABORTWARNING
  !define MUI_WELCOMEPAGE_TEXT "RamseyX 计划是一个为帮助解决图论中著名的拉姆齐问题而发起的公益分布式运算项目。\r\n\r\n我们因此能够搭建起一张运算能力惊人的巨大运算网，形成一台虚拟的“超级计算机”，这样巨大的运算力将对拉姆齐问题的探索与解决提供极大帮助。\r\n\r\n对于 RamseyX 计划，无论微小或是巨大，每个人的帮助都弥足珍贵。\r\n\r\n我们需要您的帮助！"
  !insertmacro MUI_PAGE_WELCOME

  !insertmacro MUI_PAGE_INSTFILES
  !define MUI_FINISHPAGE_RUN "$INSTDIR\RamseyX.exe"
  !insertmacro MUI_PAGE_FINISH
  !insertmacro MUI_UNPAGE_INSTFILES
  
  !insertmacro MUI_LANGUAGE "SimpChinese"
  VIProductVersion "4.4.1.0"
  VIAddVersionKey /LANG=${1033-English} "ProductName" "RamseyX 运算客户端 安装程序"
  VIAddVersionKey /LANG=${1033-English} "Comments" "Follow us at http://www.ramseyx.org/index.html"
  VIAddVersionKey /LANG=${1033-English} "CompanyName" "RamseyX 小组"
  VIAddVersionKey /LANG=${1033-English} "FileDescription" "RamseyX 运算客户端"
  VIAddVersionKey /LANG=${1033-English} "FileVersion" "4.4.1.0"

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
MessageBox MB_OK|MB_ICONEXCLAMATION "卸载前请您确保RamseyX程序已关闭，谢谢！"
Delete "$INSTDIR\*.*"
RMDir "$INSTDIR"
;DeleteRegKey ${PRODUCT_UNINST_ROOT_KRY} "${PRODUCT_UNINST_KEY}"
SectionEnd

Function un.onInit
MessageBox MB_YESNO "您真的要卸载本运算客户端吗？若有不足之处我们期待您的反馈！" IDYES NoAbort
Abort
NoAbort:
FunctionEnd

Function .onInit
InitPluginsDir
System::Call 'kernel32::CreateMutexA(i 0, i 0, t "nsis") i .rl ?e'
pop $R0
StrCmp $R0 0 +3
MessageBox MB_OK|MB_ICONEXCLAMATION "有另一个安装程序正在运行。"
Abort
FunctionEnd
