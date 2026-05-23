; Inno Setup script for SsalMuk Windows single-click installer
[Setup]
AppName=SsalMuk
AppVersion=0.1
DefaultDirName={pf}\SsalMuk
DefaultGroupName=SsalMuk
OutputBaseFilename=SsalMuk-Installer
Compression=lzma
SolidCompression=yes
DisableDirPage=yes
DisableProgramGroupPage=no
OutputDir=output
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin

[Files]
Source: "build\Release\SsalMuk.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\Release\SsalMukUI.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\Release\SsalMukInstaller.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "README.md"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\SsalMuk"; Filename: "{app}\SsalMuk.exe"
Name: "{group}\SsalMuk UI"; Filename: "{app}\SsalMukUI.exe"
Name: "{group}\Uninstall SsalMuk"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\SsalMuk.exe"; Description: "Launch SsalMuk"; Flags: nowait postinstall skipifsilent
