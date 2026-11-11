@echo off
chcp 65001 >nul
setlocal
cd /d "%~dp0"
if not exist "%~dp0support\installer\Build.ps1" (
  echo.
  echo === 압축 파일 안에서 바로 실행하셨거나 일부만 풀렸습니다. ZIP 전체를 새 폴더에 풀고 그 폴더의 설치.cmd 를 실행해 주세요 ===
  echo.
  pause
  exit /b 1
)
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0support\installer\Build.ps1"
if errorlevel 1 (
  echo.
  echo === 설치기를 열지 못했습니다. 위 글자를 복사해 알려 주세요. 같은 폴더의 INSTALL.cmd 로도 설치할 수 있습니다 ===
  pause
  exit /b 1
)
exit /b 0
