$ErrorActionPreference = 'Stop'
$resolver = Join-Path $PSScriptRoot 'Get-PackageVersion.ps1'
foreach ($case in @(
    @{ Kind = 'NuGet'; Version = '1.2.3' },
    @{ Kind = 'NuGet'; Version = '1.2.3-rc.1' },
    @{ Kind = 'Store'; Version = '2026.1011.24.0' },
    @{ Kind = 'Store'; Version = '65535.65535.65535.0' }
)) {
    $actual = & $resolver @case -RequireExplicit
    if ($actual -ne $case.Version) { throw "Unexpected normalized version: $actual" }
}
$ciVersion = & $resolver -Kind NuGet -RunNumber 24 -RunAttempt 2
if ($ciVersion -ne '0.0.0-ci.24.2') { throw "Invalid CI version: $ciVersion" }
$previewVersion = & $resolver -Kind Store -RunNumber 24
if ($previewVersion -notmatch '^\d+\.\d+\.24\.0$') { throw "Invalid preview version: $previewVersion" }
foreach ($case in @(
    @{ Kind = 'NuGet'; Version = '' },
    @{ Kind = 'NuGet'; Version = '1.2' },
    @{ Kind = 'NuGet'; Version = '01.2.3' },
    @{ Kind = 'NuGet'; Version = '1.2.3-01' },
    @{ Kind = 'NuGet'; Version = '1.2.3+build' },
    @{ Kind = 'NuGet'; Version = '1.2.3; echo injected' },
    @{ Kind = 'Store'; Version = '' },
    @{ Kind = 'Store'; Version = '1.2.3.4' },
    @{ Kind = 'Store'; Version = '1.2.65536.0' },
    @{ Kind = 'Store'; Version = '0.1.2.0' },
    @{ Kind = 'Store'; Version = '1.02.3.0' }
)) {
    $rejected = $false
    try { $null = & $resolver @case -RequireExplicit } catch { $rejected = $true }
    if (!$rejected) { throw "Unexpectedly accepted $($case.Kind) version: $($case.Version)" }
}
Write-Host 'PASS: explicit release versions, CI/preview defaults, and invalid input rejection.'
