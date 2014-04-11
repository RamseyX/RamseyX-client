;NSIS for RamseyX, ModernUI, Unicode
 !include "MUI2.nsh"

  Name "RamseyX"
  OutFile "RamseyX_5_0_3.exe"
  InstallDir "$PROGRAMFILES\RamseyX"
  InstallDirRegKey HKCU "Software\RamseyX" ""
  RequestExecutionLevel administrator

