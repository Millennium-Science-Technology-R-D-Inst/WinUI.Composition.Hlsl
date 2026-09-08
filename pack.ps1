param([switch]$Offline)
$ErrorActionPreference="Stop"
& "$PSScriptRoot\build.ps1" -Configuration Release -Offline:$Offline
if($LASTEXITCODE){exit $LASTEXITCODE}
$arguments=@("pack","$PSScriptRoot\projection\WinUI.Composition.Hlsl.Projection.csproj","-c","Release","-p:NativeConfiguration=Release","-p:NuGetAudit=false")
if($Offline){$arguments+=@("--source","$env:USERPROFILE\.nuget\packages","-p:RestorePackagesPath=$env:USERPROFILE\.nuget\packages")}
& dotnet @arguments
exit $LASTEXITCODE
