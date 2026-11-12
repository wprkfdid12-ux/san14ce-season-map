# 계절 맵 설치기 만들기·실행: Windows 기본 .NET Framework C# 컴파일러(csc.exe)로 SeasonSetup.cs 를 컴파일해서 연다.
# 게임 파일은 여기서 건드리지 않는다(설치·제거는 설치기 창에서).
param([string]$GameDirectory='',[string]$AfterUpdate='')
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
try {
    if ([string]::IsNullOrEmpty($here)) { $here = Split-Path -Parent $MyInvocation.MyCommand.Path }
    $src = Join-Path $here 'SeasonSetup.cs'
    if (-not (Test-Path -LiteralPath $src)) { throw ('설치기 자료가 없습니다: ' + $src + "`r`nZIP 전체를 새 폴더에 풀었는지 확인해 주세요.") }
    $root = Split-Path -Parent (Split-Path -Parent $here)
    $csc = $null
    foreach ($c in @((Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'), (Join-Path $env:WINDIR 'Microsoft.NET\Framework\v4.0.30319\csc.exe'))) { if (Test-Path -LiteralPath $c) { $csc = $c; break } }
    if ($null -eq $csc) { throw '.NET Framework C# 컴파일러(csc.exe)를 찾을 수 없습니다. Windows 기능 켜기/끄기에서 .NET Framework 4.8 을 켜 주세요.' }
    $out = Join-Path $here 'SeasonSetup.exe'
    if (Test-Path -LiteralPath $out) {
        # v26k50: right after an update in the same folder the old installer is still closing - wait for it (up to 30 s)
        $tries = 1; if (-not [string]::IsNullOrEmpty($AfterUpdate)) { $tries = 60 }
        for ($i = 1; $i -le $tries; $i++) {
            try { Remove-Item -LiteralPath $out -Force; break }
            catch { if ($i -ge $tries) { throw ('계절 맵 설치기가 이미 켜져 있습니다. 켜져 있는 설치기 창을 닫고 설치.cmd 를 다시 실행해 주세요. (원인: ' + $_.Exception.Message + ')') }; Start-Sleep -Milliseconds 500 }
        }
    }
    $cargs = @('/nologo','/codepage:65001','/optimize+','/target:winexe','/platform:anycpu',
        '/reference:System.dll','/reference:System.Core.dll','/reference:System.Drawing.dll','/reference:System.Windows.Forms.dll',
        '/reference:System.IO.Compression.dll','/reference:System.IO.Compression.FileSystem.dll','/reference:System.Web.Extensions.dll',
        ('/out:' + $out), $src)
    $ico = Join-Path $here 'season.ico'
    if (Test-Path -LiteralPath $ico) { $cargs = @(('/win32icon:' + $ico)) + $cargs }
    $buildOutput = & $csc @cargs 2>&1
    $code = $LASTEXITCODE
    if ($code -ne 0 -or -not (Test-Path -LiteralPath $out)) { throw ('설치기 만들기 실패(게임 파일은 바꾸지 않았습니다). 종료 코드: ' + $code + "`r`n" + ($buildOutput -join "`r`n")) }
    $bad = Join-Path $here 'BUILD_ERROR.txt'; if (Test-Path -LiteralPath $bad) { Remove-Item -LiteralPath $bad -Force -ErrorAction SilentlyContinue }
    $argList = @()
    if (-not [string]::IsNullOrEmpty($GameDirectory)) { $argList += @('--game', ('"' + $GameDirectory + '"')) }
    if (-not [string]::IsNullOrEmpty($AfterUpdate)) { $argList += @('--after-update', ('"' + $AfterUpdate + '"')) }
    if ($argList.Count -gt 0) { Start-Process -FilePath $out -WorkingDirectory $root -ArgumentList $argList | Out-Null }
    else { Start-Process -FilePath $out -WorkingDirectory $root | Out-Null }
    exit 0
} catch {
    $msg = $_.Exception.Message + "`r`n(설치기 위치: " + $here + ")"
    try { [System.IO.File]::WriteAllText((Join-Path $here 'BUILD_ERROR.txt'), $msg, [System.Text.Encoding]::UTF8) } catch {}
    Write-Host $msg
    try {
        Add-Type -AssemblyName System.Windows.Forms | Out-Null
        $hint = "계절 맵 설치기를 열지 못했습니다. 게임 파일은 바꾸지 않았습니다.`r`n`r`n" +
            "자주 있는 원인과 해결:`r`n" +
            " 1) 설치기 창이 이미 켜져 있으면 그 창을 닫고 다시 실행`r`n" +
            " 2) ZIP 을 전부 같은 '새 폴더'에 풀었는지 확인 (Program Files·OneDrive 폴더는 피하기)`r`n" +
            " 3) 백신이 설치기(SeasonSetup.exe)를 막았으면 그 폴더를 백신 예외에 넣고 다시 실행`r`n" +
            " 4) 설치기를 못 열어도 같은 폴더의 INSTALL.cmd 로 설치할 수 있습니다`r`n`r`n" +
            "아래 글자를 전부 복사해 댓글로 알려 주시면 원인을 찾습니다.`r`n`r`n---- 오류 전문 ----`r`n" + $msg
        $f = New-Object System.Windows.Forms.Form
        $f.Text = '계절 맵 - 설치기 시작 실패'; $f.Width = 820; $f.Height = 560; $f.StartPosition = 'CenterScreen'
        $t = New-Object System.Windows.Forms.TextBox
        $t.Multiline = $true; $t.ReadOnly = $true; $t.ScrollBars = 'Both'; $t.Dock = 'Fill'; $t.Font = New-Object System.Drawing.Font('Malgun Gothic', 10); $t.Text = $hint
        $b = New-Object System.Windows.Forms.Button; $b.Text = '전부 복사'; $b.Dock = 'Bottom'; $b.Height = 36
        $b.Add_Click({ [System.Windows.Forms.Clipboard]::SetText($t.Text); $b.Text = '복사됨' })
        $c = New-Object System.Windows.Forms.Button; $c.Text = '닫기'; $c.Dock = 'Bottom'; $c.Height = 36; $c.Add_Click({ $f.Close() })
        $f.Controls.Add($t); $f.Controls.Add($b); $f.Controls.Add($c)
        [void]$f.ShowDialog()
    } catch {}
    exit 1
}
