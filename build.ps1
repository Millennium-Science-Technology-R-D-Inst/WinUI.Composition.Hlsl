param([switch]$Offline, [ValidateSet("Debug","Release")][string]$Configuration="Debug", [ValidateSet("x64","Win32","ARM64")][string]$Platform="x64")
$ErrorActionPreference = 'Stop'
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$msbuild = & $vswhere -latest -prerelease -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
if (!$msbuild) { throw 'Visual Studio MSBuild not found.' }
$start = [System.Diagnostics.ProcessStartInfo]::new($msbuild)
$start.UseShellExecute = $false
$start.WorkingDirectory = $PSScriptRoot
$start.Environment.Clear()
foreach ($entry in [Environment]::GetEnvironmentVariables().GetEnumerator()) { $start.Environment[$entry.Key] = $entry.Value }
$start.Arguments = 'src\WinUI.Composition.Hlsl\WinUI.Composition.Hlsl.vcxproj /restore /p:NuGetAudit=false /p:Configuration=Debug /p:Platform=x64 /m /v:minimal /nologo /fl /flp:logfile=compile.log;verbosity=minimal'
if ($Offline) { $start.Arguments += " /p:RestoreSources=$env:USERPROFILE\.nuget\packages /p:RestorePackagesPath=$env:USERPROFILE\.nuget\packages" }
$start.Arguments = $start.Arguments.Replace("/p:Configuration=Debug","/p:Configuration=$Configuration")
$start.Arguments = $start.Arguments.Replace("/p:Platform=x64", "/p:Platform=$Platform").Replace("logfile=compile.log", "logfile=compile-$Platform.log")
$process = [System.Diagnostics.Process]::Start($start)
$process.WaitForExit()
exit $process.ExitCode



