@echo off
chcp 65001 >nul
title SAN14 Season Map v26 Install
if not exist "%~dp0tools\Season.ps1" (
  echo.
  echo === 압축 파일 안에서 바로 실행하셨습니다. 압축을 먼저 폴더에 풀고, 푼 폴더 안의 파일을 실행해 주세요 ===
  echo.
  pause
  exit /b 1
)
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\Terrain26.ps1"
if errorlevel 1 echo === 산 모양은 건너뜁니다 (게임 파일은 그대로). 계절 맵 설치를 계속합니다 ===
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\Season.ps1" -Mode Install
if %errorlevel% neq 0 goto fail
echo.
echo === 설치가 모두 끝났습니다 ===
goto end
:fail
echo.
echo === 설치 중 멈췄습니다. Claude 에게 멈췄다고만 알려 주세요 (기록은 게임 폴더에 있습니다) ===
:end
echo.
pause
