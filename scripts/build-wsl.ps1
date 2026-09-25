$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$linuxRoot = (& wsl.exe wslpath -a $repoRoot).Trim()
if (-not $linuxRoot) { throw 'WSL is not available. Install or enable WSL2 with an Ubuntu distribution.' }
$quotedRoot = $linuxRoot -replace "'", "'\''"
$command = "mkdir -p '$quotedRoot/build' && g++ -std=c++17 -O2 -Wall -Wextra -pedantic '$quotedRoot/cpp/Cube.cpp' '$quotedRoot/cpp/Solver.cpp' '$quotedRoot/cpp/main.cpp' -o '$quotedRoot/build/solver-wsl' && g++ -std=c++17 -O2 '$quotedRoot/cpp/Cube.cpp' '$quotedRoot/cpp/Solver.cpp' '$quotedRoot/tests/cube_tests.cpp' -o '$quotedRoot/build/cube-tests-wsl'"
& wsl.exe bash -lc $command
if ($LASTEXITCODE -ne 0) { throw "WSL C++ build failed with exit code $LASTEXITCODE." }
Write-Output "Built WSL solver at $linuxRoot/build/solver-wsl"
