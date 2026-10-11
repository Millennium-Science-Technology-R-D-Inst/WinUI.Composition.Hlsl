param(
    [Parameter(Mandatory)][ValidateSet('NuGet', 'Store')][string]$Kind,
    [AllowEmptyString()][string]$Version = '',
    [int]$RunNumber = 0,
    [int]$RunAttempt = 1,
    [switch]$RequireExplicit
)
$ErrorActionPreference = 'Stop'
$Version = $Version.Trim()
if ([string]::IsNullOrWhiteSpace($Version)) {
    if ($RequireExplicit) { throw 'A release package version must be supplied.' }
    if ($RunNumber -lt 1 -or $RunAttempt -lt 1) { throw 'A positive run number and attempt are required for CI versions.' }
    if ($Kind -eq 'NuGet') {
        $Version = "0.0.0-ci.$RunNumber.$RunAttempt"
    } else {
        $Version = "$(Get-Date -Format yyyy).$([int](Get-Date -Format MMdd)).$RunNumber.0"
    }
}
if ($Kind -eq 'NuGet') {
    # Use the same parser/normalization as NuGet pack and restore, including
    # four-part versions. The SDK ships this assembly on Windows and Linux.
    if (-not ('NuGet.Versioning.NuGetVersion' -as [type])) {
        $sdks = @(& dotnet --list-sdks)
        if ($LASTEXITCODE -ne 0) { throw 'Unable to locate the .NET SDK.' }
        $sdk = $sdks | Select-Object -Last 1
        if ($sdk -notmatch '^(\S+) \[(.+)\]$') { throw 'A .NET SDK is required to parse NuGet versions.' }
        $assembly = Join-Path (Join-Path $Matches[2] $Matches[1]) 'NuGet.Versioning.dll'
        Add-Type -Path $assembly
    }
    $parsed = [NuGet.Versioning.NuGetVersion]::Parse($Version)
    $Version = $parsed.ToNormalizedString()
} else {
    # MSIX Identity.Version contains four UInt16 components. Three-part release
    # inputs get a zero revision; explicitly supplied revisions are preserved.
    if ($Version -notmatch '^[0-9]+\.[0-9]+\.[0-9]+(\.[0-9]+)?$') {
        throw 'MSIX version must have three or four numeric components.'
    }
    $parts = $Version.Split('.') | ForEach-Object { [long]$_ }
    if ($parts[0] -lt 1 -or @($parts | Where-Object { $_ -gt 65535 }).Count) {
        throw 'MSIX version components must be 0..65535 and Major must be positive.'
    }
    if ($parts.Count -eq 3) { $parts += 0 }
    $Version = $parts -join '.'
}
$Version
