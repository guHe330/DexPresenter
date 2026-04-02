#ifndef MyAppVersion
  #define MyAppVersion "1.0.0"
#endif

#define MyAppName "DexPresenter"
#define MyAppPublisher "DexPresenter"
#define MyAppExeName "DexPresenter.exe"

[Setup]
AppId={{A1B2C3D4-5E6F-7890-ABCD-EF1234567890}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputDir=output
OutputBaseFilename=DexPresenter_{#MyAppVersion}_Setup
Compression=lzma2
SolidCompression=yes
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
WizardStyle=modern
MinVersion=10.0.17763
UsePreviousAppDir=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "..\..\build\DexPresenter.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\build\DexPresenter_Sources.zip"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"

[Registry]
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; \
  ValueType: string; ValueName: "DexPresenter"; \
  ValueData: """{app}\DexPresenter.exe"""; \
  Flags: uninsdeletevalue

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Launch DexPresenter"; \
  Flags: nowait postinstall skipifsilent

[Code]
var g_KeepConfig: Boolean;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var ResultCode: Integer;
begin
  if CurUninstallStep = usUninstall then begin
    Exec('taskkill', '/F /IM DexPresenter.exe', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    g_KeepConfig := MsgBox(
      'Keep your DexPresenter settings?' + #13#10 +
      'Settings are stored in: ' + ExpandConstant('{userappdata}') + '\DexPresenter',
      mbConfirmation, MB_YESNO) = IDYES;
  end;
  if CurUninstallStep = usPostUninstall then begin
    if not g_KeepConfig then
      DelTree(ExpandConstant('{userappdata}\DexPresenter'), True, True, True);
  end;
end;
