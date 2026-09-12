param([ValidateSet("Cpp","CSharp")][string]$Language, [ValidateSet("x64","Win32","ARM64")][string]$Platform="x64")
$ErrorActionPreference="Stop"
$vswhere=[Environment]::GetFolderPath('ProgramFilesX86')+'\Microsoft Visual Studio\Installer\vswhere.exe'
$msbuild=& $vswhere -latest -prerelease -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
$root=Split-Path $PSScriptRoot -Parent
$start=[Diagnostics.ProcessStartInfo]::new($msbuild)
$start.UseShellExecute=$false;$start.WorkingDirectory=$root
$start.Environment.Clear()
foreach($entry in [Environment]::GetEnvironmentVariables().GetEnumerator()){$start.Environment[$entry.Key]=$entry.Value}
$project=if($Language -eq "Cpp"){"tests\Cpp\WUILiquidGlassDemo.Hlsl.vcxproj"}else{"tests\CSharp\HlslCSharpConsumer.csproj"}
$start.Arguments="$project /restore /p:Configuration=Debug /p:Platform=$Platform /p:RestorePackagesPath=$env:USERPROFILE\.nuget\packages /p:NuGetAudit=false /m /v:minimal /nologo /fl /flp:logfile=$Language-$Platform-test-build.log;verbosity=minimal"
$process=[Diagnostics.Process]::Start($start);$process.WaitForExit();exit $process.ExitCode
