
$ErrorActionPreference = "Stop"

$QtPath = "C:\Qt\6.12.0\msvc2022_64"
$BuildDir = "build"
$DistDir = "dist"

$QtConfig = Join-Path $QtPath "lib\cmake\Qt6\Qt6Config.cmake"
$WinDeployQt = Join-Path $QtPath "bin\windeployqt.exe"

if (!(Test-Path $QtConfig)) {
    Write-Host "Qt6Config.cmake not found!" -ForegroundColor Red
    Write-Host "Check Qt installation: $QtPath"
    exit 1
}

if (!(Test-Path $WinDeployQt)) {
    Write-Host "windeployqt.exe not found!" -ForegroundColor Red
    exit 1
}

Write-Host "==> Cleaning build and dist directories..." -ForegroundColor Blue

foreach ($Dir in @($BuildDir, $DistDir)) {
    if (Test-Path $Dir) {
        Remove-Item -Recurse -Force $Dir
    }
}

Write-Host "==> Configuring CMake (MSVC Release)..." -ForegroundColor Blue

cmake -S . -B $BuildDir `
    -G "Visual Studio 17 2022" `
    -A x64 `
    -DCMAKE_PREFIX_PATH="$QtPath" `
    -DCMAKE_INSTALL_PREFIX="$DistDir"

if ($LASTEXITCODE -ne 0) {
    Write-Host "==> CMake configuration failed." -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "==> Building Release..." -ForegroundColor Blue

cmake --build $BuildDir --config Release --parallel

if ($LASTEXITCODE -ne 0) {
    Write-Host "==> Build failed." -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "==> Installing application..." -ForegroundColor Blue

cmake --install $BuildDir --config Release --prefix $DistDir

if ($LASTEXITCODE -ne 0) {
    Write-Host "==> Installation failed." -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "==> Deploying Qt dependencies..." -ForegroundColor Blue

$ExePath = Join-Path $DistDir "bin\BinaryInspector.exe"

if (!(Test-Path $ExePath)) {
    Write-Host "BinaryInspector.exe not found in $ExePath" -ForegroundColor Red
    exit 1
}

& $WinDeployQt --release --compiler-runtime $ExePath

if ($LASTEXITCODE -ne 0) {
    Write-Host "==> Qt deployment failed." -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host " BinaryInspector Release ready!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Executable: $ExePath" -ForegroundColor Cyan
Write-Host "Distribution: $DistDir\bin" -ForegroundColor Cyan
Write-Host ""
Write-Host "Run BinaryInspector.exe to start."