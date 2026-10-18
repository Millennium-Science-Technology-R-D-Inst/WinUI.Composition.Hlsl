param(
    [Parameter(Mandatory)][string]$PackageVersion,
    [switch]$CppDemo
)
$ErrorActionPreference = 'Stop'

foreach ($id in @('WinUI.Composition.Hlsl', 'WinUI.LiquidGlass')) {
    $package = Join-Path $PSScriptRoot "../artifacts/packages/$id.$PackageVersion.nupkg"
    if (!(Test-Path $package)) { throw "Validated package was not downloaded: $package" }
}

if ($CppDemo) {
    $projectPath = Join-Path $PSScriptRoot '../tests/Cpp/WUILiquidGlassDemo.Hlsl.vcxproj'
    [xml]$project = Get-Content $projectPath -Raw
    $ns = [Xml.XmlNamespaceManager]::new($project.NameTable)
    $ns.AddNamespace('msb', $project.DocumentElement.NamespaceURI)
    $projectRefs = @($project.SelectNodes('//msb:ProjectReference', $ns))
    if ($projectRefs.Count -ne 2) { throw 'Expected core and LiquidGlass project references in the Demo.' }
    foreach ($reference in $projectRefs) {
        [void]$reference.ParentNode.RemoveChild($reference)
    }
    $group = $project.CreateElement('ItemGroup', $project.DocumentElement.NamespaceURI)
    foreach ($id in @('WinUI.Composition.Hlsl', 'WinUI.LiquidGlass')) {
        $reference = $project.CreateElement('PackageReference', $project.DocumentElement.NamespaceURI)
        $reference.SetAttribute('Include', $id)
        $reference.SetAttribute('VersionOverride', $PackageVersion)
        [void]$group.AppendChild($reference)
    }
    [void]$project.DocumentElement.AppendChild($group)
    $project.Save($projectPath)
}

# An isolated cache and source mapping prevent an already published release with
# the same version from taking precedence over the packages produced by this run.
@'
<?xml version="1.0" encoding="utf-8"?>
<configuration>
  <config>
    <add key="globalPackagesFolder" value="artifacts/nuget-cache" />
  </config>
  <packageSources>
    <clear />
    <add key="LocalPackages" value="artifacts/packages" />
    <add key="nuget.org" value="https://api.nuget.org/v3/index.json" />
  </packageSources>
  <packageSourceMapping>
    <clear />
    <packageSource key="LocalPackages">
      <package pattern="WinUI.Composition.Hlsl" />
      <package pattern="WinUI.LiquidGlass" />
    </packageSource>
    <packageSource key="nuget.org">
      <package pattern="*" />
    </packageSource>
  </packageSourceMapping>
</configuration>
'@ | Set-Content (Join-Path $PSScriptRoot '../NuGet.CI.Config') -Encoding utf8
