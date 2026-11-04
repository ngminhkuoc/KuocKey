@echo off
rem Runs the live typing test against the running KuocKey; output to Z:\KuocKey\Windows\Tests\livetest.out
C:\vk\build\App\kuockey_livetest.exe C:\vk\src\Windows\Tests\cases.tsv > Z:\KuocKey\Windows\Tests\livetest.out 2>&1
echo EXIT %ERRORLEVEL% >> Z:\KuocKey\Windows\Tests\livetest.out
