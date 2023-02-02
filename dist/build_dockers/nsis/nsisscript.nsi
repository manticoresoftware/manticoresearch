; The name of the installer
Name "Manticore Search"

; The file to write
OutFile "manticore_installer.exe"

; Request application privileges for Windows Vista and higher
;RequestExecutionLevel admin

; Build Unicode installer
Unicode True

; The default installation directory
InstallDir $PROGRAMFILES\Manticore

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "SOFTWARE\Manticore Software LTD" "manticore"

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

Function unpackInstall
  Pop $R0  ; txt file name
  Pop $R1  ; archive name
  FileOpen $0 $INSTDIR\$R0 r
  IfErrors Fail1
  FileRead $0 $1
  DetailPrint $1
  FileClose $0
  NSISdl::download $1 $R1
  Pop $0
  StrCmp $0 success success1
  SetDetailsView show
  DetailPrint "Error! Unable to download $R1!"
  Abort

success1:
  Delete $INSTDIR\$R0
  nsisunz::Unzip "$INSTDIR\$R1" "$INSTDIR"
  Pop $0
  StrCmp $0 success success2
  DetailPrint "Error! Unable to unzip $R1!"

success2:
  Delete $INSTDIR\$R1
  Goto Ok

Fail1:
  MessageBox MB_OK "Error! Unable to read $R0!"
  Abort

Ok:
FunctionEnd


Section "Manticore Search"
  SectionIn RO
  SetOutPath $INSTDIR

  File "manticore_src.txt"
  Push "manticore.zip"
  Push "manticore_src.txt"
  Call unpackInstall

  CreateDirectory "$INSTDIR\var\data"
  CreateDirectory "$INSTDIR\var\log\manticore"
  CreateDirectory "$INSTDIR\var\run\manticore"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM "SOFTWARE\Manticore Software LTD" "manticore" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Manticore" "DisplayName" "Manticore Search"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Manticore" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Manticore" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Manticore" "NoRepair" 1

  WriteUninstaller "$INSTDIR\uninstall.exe"  
SectionEnd

Section "Manticore Executor"
  File "executor_src.txt"
  Push "executor.zip"
  Push "executor_src.txt"
  Call unpackInstall

  File "buddy_src.txt"
  Push "buddy.zip"
  Push "buddy_src.txt"
  Call unpackInstall
SectionEnd

Section "Manticore Columnar Library"
  File "mcl_src.txt"
  Push "mcl.zip"
  Push "mcl_src.txt"
  Call unpackInstall
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Manticore"
  DeleteRegKey HKLM "SOFTWARE\Manticore Software LTD"

  ; Remove files and uninstaller
  RmDir /r $INSTDIR\bin
  RmDir /r $INSTDIR\etc
  RmDir /r $INSTDIR\include
  RmDir /r $INSTDIR\share
  RmDir /r $INSTDIR\__MACOSX
  Delete $INSTDIR\uninstall.exe

  ; Remove directories
  RMDir "$SMPROGRAMS\Manticore"
  RMDir "$INSTDIR"

SectionEnd
