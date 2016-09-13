Name "Gamebase Manager"

OutFile "InstallGamebaseManager.exe"
LicenseData "manager\LICENSE.txt"
InstallDir "C:\MyPrograms\"

Page license
Page directory
Page instfiles

Section "Installation"
  SectionIn RO
  
  CreateDirectory "$INSTDIR"
  SetOutPath "$INSTDIR"
  File /R "manager"
  SetOutPath "$INSTDIR\manager"
  CreateShortcut "$INSTDIR\Manager.lnk" "$INSTDIR\manager\manager.exe"
SectionEnd
