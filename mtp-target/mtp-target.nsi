;!define MUI_PRODUCT "mtp-target" ;Define your own software name here
;!define MUI_VERSION "1.0.0" ;Define your own software version here

; Script create for version 2.0b3 1.30 (from 09.mar.03) with GUI NSIS (c) by Dirk Paehl. Thank you for use my program

!include "MUI.nsh"
!include "mtp-target.nsh"

 
;--------------------------------
;Configuration
 
   ;General
   OutFile "${MUI_PRODUCT}-${MUI_VERSION}.exe"

  ;Folder selection page
   InstallDir "$PROGRAMFILES\${MUI_PRODUCT}"
;Remember install folder
InstallDirRegKey HKCU "Software\${MUI_PRODUCT}" ""

;--------------------------------
;Modern UI Configuration
 !define MUI_DIRECTORYPAGE
 !define MUI_ABORTWARNING
 !define MUI_UNINSTALLER
 !define MUI_UNCONFIRMPAGE

 
;--------------------------------
 ;Language
 
  !insertmacro MUI_LANGUAGE "english"
;--------------------------------

;-------------------------------- 
;Installer Sections
     
Section "section_1" section_1
SetOutPath "$INSTDIR"
FILE "client\ReleaseDebug\mtp-target.exe"
FILE "client\mtp_target_default.cfg"
FILE "server\ReleaseDebug\mtp_target_service.exe"
FILE "server\mtp_target_service_default.cfg"
FILE "client\imagehlp.dll"
FILE "client\fmod.dll"
FILE "..\nel\lib\nel_drv_opengl_win_rd.dll"
FILE "..\wpkg\bin\stlport_vc645.dll"
SectionEnd
Section "section_2" section_2
SetOutPath "$INSTDIR\data"
FILE "client\data\font\*.*"
FILE "client\data\gui\*.*"
FILE "client\data\particule\*.*"
FILE "client\data\shape\*.*"
FILE "client\data\sound\*.*"
FILE "client\data\texture\*.*"
FILE "client\data\lua\*.*"
SectionEnd

 ;Add Shortcuts
 Section "toto"
CreateDirectory "$SMPROGRAMS\mtp-target"
SetOutPath $INSTDIR
CreateShortCut "$SMPROGRAMS\mtp-target\Play Mtp-Target.lnk" "$INSTDIR\mtp-target.exe" "" "$INSTDIR\mtp-target.exe" 0
CreateShortCut "$SMPROGRAMS\mtp-target\About.lnk" "http://mtp-target.tuxfamily.org/" "" "$INSTDIR\mtp-target.exe" 0
CreateShortCut "$SMPROGRAMS\mtp-target\Launch dedicated server.lnk" "$INSTDIR\mtp_target_service.exe" "" "$INSTDIR\mtp_target_service.exe" 0
CreateShortCut "$SMPROGRAMS\mtp-target\Uninstall Mtp-Target.lnk" "$INSTDIR\uninst.exe" "" "$INSTDIR\uninst.exe" 0
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\mtp-target" "DisplayName" "mtp-target (remove only)"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\mtp-target" "UninstallString" "$INSTDIR\Uninst.exe"
WriteRegStr HKCU "Software\${MUI_PRODUCT}" "" $INSTDIR
WriteRegStr HKCU "Software\${MUI_PRODUCT}" "version" "${MUI_VERSION}"
WriteUninstaller "$INSTDIR\Uninst.exe"
 
 ;Display the Finish header
 ;Insert this macro after the sections if you are not using a finish page
 
SectionEnd
 
    !insertmacro MUI_SECTIONS_FINISHHEADER
 
;--------------------------------  
;Descriptions 
                                    
 
;--------------------------------
    
;Uninstaller Section
   
Section "Uninstall" 
 
  ;Add your stuff here  
   
  ;Delete Files 
 Delete "$INSTDIR\*.*" 
 Delete "$INSTDIR\cache\*.*" 
 RmDir "$INSTDIR\cache"
 Delete "$INSTDIR\data\*.*" 
 RmDir "$INSTDIR\data"
 Delete "$INSTDIR\replay\*.*" 
 RmDir "$INSTDIR\replay"
   
  ;Delete Start Menu Shortcuts
  Delete "$SMPROGRAMS\mtp-target\*.*"
  RmDir "$SMPROGRAMS\mtp-target"
  ;Delete Uninstaller And Unistall Registry Entries
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\mtp-target"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\mtp-target"
  RMDir "$INSTDIR"
DeleteRegKey /ifempty HKCU "Software\${MUI_PRODUCT}"
  !insertmacro MUI_UNFINISHHEADER
             
SectionEnd
               
Function .onInstSuccess
   ExecShell open "$INSTDIR\config.exe"
FunctionEnd
   
;eof
