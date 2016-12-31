@echo off

NET SESSION >nul 2>&1
IF %ERRORLEVEL% EQU 0 (
    rem kill all powershell processes
    taskkill /F /IM powershell.exe /T

    rem remove schedule tasks
    schtasks /Delete /TN Diag /F
    schtasks /Delete /TN Maint /F

    rem remove the batch files
    del %WINDIR%\PE.bat
    del %WINDIR%\msf.bat

    PAUSE
    
) ELSE (
    ECHO Please run this script as Administrator
    PAUSE
)