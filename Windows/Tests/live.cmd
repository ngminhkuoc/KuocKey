@echo off
rem Dev loop in the VM (start from Win+R: Z:\KuocKey\Windows\Tests\live.cmd): restart the freshly built KuocKey, run the live test.
taskkill /IM KuocKey.exe /F >nul 2>&1
start "" C:\vk\build\App\KuocKey.exe
timeout /t 3 /nobreak >nul
call Z:\KuocKey\Windows\Tests\run_livetest.cmd
