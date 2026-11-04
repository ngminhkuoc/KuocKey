; KuocKey for Windows installer (Inno Setup 6). Per-user, no admin prompt.
;   iscc /DVersion=1.0.14 /DArch=x64 /DSrc=..\build\App KuocKey.iss
#ifndef Version
  #define Version "0.0.0"
#endif
#ifndef Arch
  #define Arch "x64"
#endif
#ifndef Src
  #define Src "..\build\App"
#endif

[Setup]
AppId={{7E2B5C1A-9D3F-4C7B-8E61-KUOCKEY0000001}
AppName=KuocKey
AppVersion={#Version}
AppVerName=KuocKey {#Version}
AppPublisher=Tony Dinh, modified by Quoc Nguyen
AppPublisherURL=https://kuockey.quocnguyen.dev
VersionInfoVersion={#Version}
DefaultDirName={localappdata}\Programs\KuocKey
DisableDirPage=yes
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
OutputDir=.\out
OutputBaseFilename=KuocKey-{#Version}-{#Arch}-Setup
SetupIconFile=..\App\KuocKey.ico
UninstallDisplayIcon={app}\KuocKey.exe
UninstallDisplayName=KuocKey
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
CloseApplications=yes
CloseApplicationsFilter=KuocKey.exe
RestartApplications=no
#if Arch == "arm64"
ArchitecturesAllowed=arm64
ArchitecturesInstallIn64BitMode=arm64
#else
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
#endif

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"

[Messages]
; the few screens people see, in Vietnamese like the app
WelcomeLabel1=Cài đặt KuocKey
WelcomeLabel2=KuocKey {#Version} sẽ được cài cho tài khoản của bạn, không cần quyền quản trị.%n%nBấm Next để tiếp tục.
FinishedHeadingLabel=Đã cài xong KuocKey
FinishedLabel=KuocKey nằm ở khay hệ thống (góc dưới bên phải). Bấm Alt+Z để chuyển Anh/Việt.
ConfirmUninstall=Gỡ KuocKey khỏi máy?

[Files]
Source: "{#Src}\KuocKey.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\KuocKey"; Filename: "{app}\KuocKey.exe"

[Registry]
; the app's own "Mở KuocKey khi đăng nhập" entry goes away with the app
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueName: "KuocKey"; Flags: uninsdeletevalue dontcreatekey

[Run]
Filename: "{app}\KuocKey.exe"; Description: "Mở KuocKey"; Flags: nowait postinstall skipifsilent

[UninstallRun]
Filename: "{sys}\taskkill.exe"; Parameters: "/IM KuocKey.exe /F"; Flags: runhidden; RunOnceId: "KillKuocKey"
