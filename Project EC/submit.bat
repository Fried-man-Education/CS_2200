@echo off

rem Windows submit script by Tanner Muldoon for CS22000

set CIRCUIT=*.sim
set "SUBMIT_FILES=%CIRCUIT%"
set SUBMISSION_NAME=ec-pipeline

where 7z
if %ERRORLEVEL% NEQ 0 (
    echo 7zip is required to package submissions on Windows
    echo You can download it from https://www.7-zip.org/
    exit /b 1
)

7z a %SUBMISSION_NAME%.tar -ssw %SUBMIT_FILES%
7z a %SUBMISSION_NAME%.tar.gz -ssw %SUBMISSION_NAME%.tar
del %SUBMISSION_NAME%.tar

echo Created submission archive %SUBMISSION_NAME%.tar.gz
