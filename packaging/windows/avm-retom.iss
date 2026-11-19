; Windows installer for AVM RETOM. Built by .gitlab-ci.yml on the "windows" runner:
;   ISCC /DMyAppVersion=1.2.3 /DDeployDir=<dir with exe + windeployqt output> /DOutDir=<dist> avm-retom.iss
#ifndef MyAppVersion
  #define MyAppVersion "0.0.0"
#endif
#ifndef DeployDir
  #define DeployDir "..\..\deploy"
#endif
#ifndef OutDir
  #define OutDir "..\..\dist"
#endif
#define Name "AVM RETOM"
#define ExeName "avm-retom.exe"

[Setup]
AppId={{6D0F3B1E-2C74-4A58-9E1B-5A7C3F0D9B26}
AppName={#Name}
AppVersion={#MyAppVersion}
AppVerName={#Name} {#MyAppVersion}
AppPublisher=AVM-Energo LLC
AppPublisherURL=https://www.avmenergo.ru
DefaultDirName={autopf}\AVM-RETOM
DefaultGroupName={#Name}
UsedUserAreasWarning=no
Compression=lzma
SolidCompression=yes
WizardStyle=modern
OutputBaseFilename=avm-retom-setup-{#MyAppVersion}
OutputDir={#OutDir}
VersionInfoVersion={#MyAppVersion}
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "{#DeployDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs

[Icons]
Name: "{group}\{#Name}"; Filename: "{app}\{#ExeName}"
Name: "{group}\{cm:UninstallProgram,{#Name}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#Name}"; Filename: "{app}\{#ExeName}"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; Flags: unchecked

[Run]
Filename: "{app}\{#ExeName}"; Description: "{cm:LaunchProgram,{#Name}}"; Flags: nowait postinstall skipifsilent
