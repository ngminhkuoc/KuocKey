# Building KuocKey for Windows

How to build `KuocKey.exe` from source and run it on your own laptop. KuocKey is a single exe (static runtime, no installer needed, no Visual C++ Redistributable).

## 1. Install the tools (once)

You need a C++ compiler, CMake, Ninja and Python. Easiest route with winget (run in PowerShell):

```powershell
winget install Microsoft.VisualStudio.2022.BuildTools --override "--quiet --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
winget install Kitware.CMake
winget install Ninja-build.Ninja
winget install Python.Python.3.12
```

Notes:
- The important part is the **Desktop development with C++** workload (MSVC + Windows SDK). If you already use Visual Studio, open the Visual Studio Installer, choose Modify, and tick that workload. A Visual Studio install with no C++ workload is not enough.
- Close and reopen the terminal after installing so `cmake`, `ninja` and `python` are on PATH.
- Python is only needed for `check.ps1` (the engine tests), not for building the app.

## 2. Open a developer shell

MSVC must be on PATH. Either:
- Start menu, search **x64 Native Tools Command Prompt for VS 2022**, then run `pwsh` in it, or
- In PowerShell:
  ```powershell
  & "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -property installationPath
  ```
  then run `Common7\Tools\Launch-VsDevShell.ps1 -Arch amd64` from the folder it prints.

Check it worked: `cl` and `cmake --version` should both print something. On an ARM laptop use the ARM64 tools prompt and `-Arch arm64`.

## 3. Build

From the repository root:

```powershell
cmake -S Windows -B Windows/build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build Windows/build --target kuockey
```

The result is `Windows\build\App\KuocKey.exe`.

To build and run all the checks (engine tests plus the Bảng gõ tắt table test) instead:

```powershell
pwsh Windows/check.ps1
```

This is slower, and ends with `ALL CHECKS PASSED` when everything is fine. Run it at least once after pulling new code, because the shortcut table code has not been compiled by its author.

Only the shortcut table test:

```powershell
cmake --build Windows/build --target kuockey_macrotest
Windows\build\App\kuockey_macrotest.exe
```

## 4. Run it

1. Quit any KuocKey that is already running (tray icon, right click, Thoát KuocKey). Only one copy can run.
2. Double click `Windows\build\App\KuocKey.exe`, or copy it somewhere permanent first, for example `C:\Users\<you>\Apps\KuocKey\KuocKey.exe`.
3. A V or E icon appears in the tray. Alt+Z switches Vietnamese and English.
4. In the main window turn on **Mở KuocKey khi đăng nhập** to start it with Windows. That registers the exe path, so put the exe in its final folder first.

Your settings (registry `HKCU\Software\KuocKey`) and shortcut list (`%LOCALAPPDATA%\KuocKey\macros.csv`) are kept between rebuilds.

## 5. Try Bảng gõ tắt

1. Tray icon, right click, **Bảng gõ tắt…** (or main window, row Bảng gõ tắt, Mở…).
2. Add `omw` with the text `On my way!`, press Thêm / Cập nhật.
3. In Notepad type `omw` then Space. It should become `On my way! `. Backspace straight after puts `omw ` back.
4. Check Vietnamese mode too (Alt+Z), and the "only in Vietnamese" toggle.
5. Try Xuất CSV and Nhập CSV with a small file: `omw,On my way!` per line.

## Troubleshooting

- **`cmake` can't find a compiler / `cl` not found:** you are not in a developer shell (step 2), or the C++ workload is missing.
- **Ninja not found:** reinstall Ninja, reopen the terminal. Alternatively drop `-G Ninja` and build with the Visual Studio generator: `cmake -S Windows -B Windows/build -A x64` then `cmake --build Windows/build --config Release --target kuockey` (the exe then lands in `Windows\build\App\Release\`).
- **Stale configuration after switching compiler or architecture:** delete `Windows\build` and configure again.
- **A compile error in the new shortcut code:** it was written without a compiler available. Send the first error and it can be fixed quickly.
- **Typing does nothing in an admin window (Task Manager, an elevated terminal):** Windows blocks a normal-user keyboard hook there. Run KuocKey as administrator if you need that.
- **Antivirus or SmartScreen warns about the exe:** it is unsigned and installs a keyboard hook, which looks suspicious to scanners. Building it yourself from this source is the safest way to be sure what it is.

## Optional: an installer

CI builds installers with Inno Setup (`Windows/installer/KuocKey.iss`). For personal use the plain exe is enough. If you want one anyway, install Inno Setup 6 and run:

```powershell
& "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" /DVersion=<version> /DArch=x64 /DSrc=<folder containing KuocKey.exe> Windows\installer\KuocKey.iss
```

`<version>` is `MARKETING_VERSION` in `project.yml`.
