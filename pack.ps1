param([switch]$Offline, [ValidateSet("x64","x86","ARM64")][string]$Platform="x64")
$ErrorActionPreference="Stop"
$vswhere=[Environment]::GetFolderPath('ProgramFilesX86')+'\Microsoft Visual Studio\Installer\vswhere.exe'
$msbuild=& $vswhere -latest -prerelease -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
if(!$msbuild){throw "Visual Studio MSBuild not found."}
$start=[Diagnostics.ProcessStartInfo]::new($msbuild)
$start.UseShellExecute=$false
$start.WorkingDirectory=$PSScriptRoot
$start.Environment.Clear()
foreach($entry in [Environment]::GetEnvironmentVariables().GetEnumerator()){$start.Environment[$entry.Key]=$entry.Value}
$start.Arguments="projection\WinUI.Composition.Hlsl.Projection.csproj /restore /t:Pack /p:Configuration=Release /p:Platform=$Platform /p:NuGetAudit=false /m /nologo /v:minimal /fl /flp:logfile=pack.log;verbosity=minimal"
if($Offline){$start.Arguments+=" /p:RestoreSources=$env:USERPROFILE\.nuget\packages /p:RestorePackagesPath=$env:USERPROFILE\.nuget\packages"}
$process=[Diagnostics.Process]::Start($start)
$process.WaitForExit()
exit $process.ExitCode
