param(
    [Parameter(Mandatory)][string]$PackagePath,
    [Parameter(Mandatory)][ValidateSet('x64', 'ARM64')][string]$Platform,
    [Parameter(Mandatory)][string]$PackageVersion
)
$ErrorActionPreference = 'Stop'

function Get-EntryHash($Archive, [string]$Path) {
    $entry = $Archive.GetEntry($Path)
    if (!$entry) { throw "Missing packaged runtime: $Path" }
    $stream = $entry.Open()
    $sha256 = [Security.Cryptography.SHA256]::Create()
    try { [Convert]::ToHexString($sha256.ComputeHash($stream)) }
    finally { $sha256.Dispose(); $stream.Dispose() }
}

$msix = [IO.Compression.ZipFile]::OpenRead((Resolve-Path $PackagePath).Path)
try {
    foreach ($id in @('WinUI.Composition.Hlsl', 'WinUI.LiquidGlass')) {
        $nugetPath = Join-Path $PSScriptRoot "../artifacts/packages/$id.$PackageVersion.nupkg"
        $nuget = [IO.Compression.ZipFile]::OpenRead((Resolve-Path $nugetPath).Path)
        try {
            $expected = Get-EntryHash $nuget "runtimes/win-$($Platform.ToLowerInvariant())/native/$id.dll"
            $actual = Get-EntryHash $msix "$id.dll"
            if ($actual -ne $expected) { throw "$Platform MSIX does not contain the validated $id runtime." }
        } finally { $nuget.Dispose() }
    }
} finally { $msix.Dispose() }
Write-Host "$Platform MSIX contains both native DLLs from this run's exact NuGet packages."
