#ifndef MyAppVersion
  #define MyAppVersion "1.0.0"
#endif

#define MyAppName "DexPresenter"
#define MyAppPublisher "DexPresenter"
#define MyAppExeName "DexPresenter.exe"

[Setup]
; Stable application identity — must never change across releases (upgrades/uninstall rely on it).
AppId={{0AB9A469-6249-44A1-A8F2-A4A54DBA3779}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputDir=output
OutputBaseFilename=DexPresenter_{#MyAppVersion}_Setup
SetupIconFile=..\..\resources\DexPresenter.ico
LicenseFile=..\..\LICENSE
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
Source: "..\..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"

; Autorun (HKCU\...\Run) is managed entirely by the app via its "Start with
; Windows" setting (off by default), so the installer intentionally does NOT
; write it. This keeps the Settings toggle and actual autorun always in sync.
; The uninstaller cleans up the value in [Code] in case the app enabled it.

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
    // Remove the autorun entry the app may have created (harmless if absent).
    RegDeleteValue(HKEY_CURRENT_USER,
      'Software\Microsoft\Windows\CurrentVersion\Run', 'DexPresenter');
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
