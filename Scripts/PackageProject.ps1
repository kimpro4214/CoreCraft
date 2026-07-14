param(
    [Parameter(Mandatory = $true)]
    [string]$Scene
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$scenePath = (Resolve-Path $Scene).Path
$msbuild = 'C:\Program Files\Microsoft Visual Studio\18\Insiders\MSBuild\Current\Bin\amd64\MSBuild.exe'

if (-not (Test-Path $msbuild)) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    $msbuild = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
}
if (-not (Test-Path $msbuild)) {
    throw 'MSBuild를 찾지 못했습니다.'
}

& $msbuild (Join-Path $root 'GameCoding.sln') /t:GameRuntime /m /p:Configuration=Release /p:Platform=x64 /v:minimal
if ($LASTEXITCODE -ne 0) {
    throw "GameRuntime 빌드 실패: $LASTEXITCODE"
}

$staging = Join-Path $root 'Builds\CoreCraft.staging'
$output = Join-Path $root 'Builds\CoreCraft'
if (Test-Path $staging) { Remove-Item -LiteralPath $staging -Recurse -Force }
New-Item -ItemType Directory -Path (Join-Path $staging 'Binaries') -Force | Out-Null

Copy-Item -LiteralPath (Join-Path $root 'Binaries\GameRuntime\Release\GameRuntime.exe') -Destination (Join-Path $staging 'Binaries')
Copy-Item -LiteralPath (Join-Path $root 'Shaders') -Destination $staging -Recurse
Copy-Item -LiteralPath (Join-Path $root 'Resources') -Destination $staging -Recurse
New-Item -ItemType Directory -Path (Join-Path $staging 'Resources\Scenes') -Force | Out-Null
Copy-Item -LiteralPath $scenePath -Destination (Join-Path $staging 'Resources\Scenes\Startup.scene.xml') -Force

if (Test-Path $output) { Remove-Item -LiteralPath $output -Recurse -Force }
Move-Item -LiteralPath $staging -Destination $output
Write-Output "Package created: $output"
