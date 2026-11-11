@echo off
chcp 65001 >nul
title SAN14 Season Map v26 Uninstall
if not exist "%~dp0tools\Season.ps1" (
  echo.
  echo === 압축 파일 안에서 바로 실행하셨습니다. 압축을 먼저 폴더에 풀고, 푼 폴더 안의 파일을 실행해 주세요 ===
  echo.
  pause
  exit /b 1
)
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\Season.ps1" -Mode Uninstall
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\Terrain26.ps1" -Uninstall
echo.
pause
