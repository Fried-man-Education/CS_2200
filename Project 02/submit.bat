@echo off

rem Windows submit script by Tanner Muldoon for CS22000

set CIRCUIT= %~dp0LC-22a.sim
set MICROCODE=%~dp0microcode\microcode.xlsx
set ASSEMBLY=%~dp0assembly/prj2.s
set "SUBMIT_FILES=%CIRCUIT% %MICROCODE% %ASSEMBLY%"
set SUBMISSION_NAME=project2-interrupts

where 7z
if %ERRORLEVEL% NEQ 0 (
    echo 7zip is required to package submissions on Windows
    echo You can download it from https://www.7-zip.org/
    exit /b 1
)

7z a %SUBMISSION_NAME%.zip -ssw %SUBMIT_FILES%  

echo Created submission archive %SUBMISSION_NAME%.zip
