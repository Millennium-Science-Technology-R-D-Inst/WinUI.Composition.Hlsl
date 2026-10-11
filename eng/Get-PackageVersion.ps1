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
    # NuGet SemVer with three numeric components and optional prerelease labels.
    # Exclude build metadata so artifact names and the normalized feed version agree.
    if ($Version -cnotmatch '^(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)(-[0-9A-Za-z-]+(\.[0-9A-Za-z-]+)*)?$') {
        throw 'NuGet version must be Major.Minor.Patch with an optional -prerelease suffix.'
    }
    foreach ($component in $Version.Split('-')[0].Split('.')) {
        if ([long]$component -gt [int]::MaxValue) { throw 'NuGet numeric components must fit a 32-bit integer.' }
    }
    if ($Version.Contains('-')) {
        foreach ($label in $Version.Substring($Version.IndexOf('-') + 1).Split('.')) {
            if ($label -match '^0[0-9]+$') { throw 'Numeric prerelease labels must not contain leading zeroes.' }
        }
    }
} else {
    if ($Version -cnotmatch '^(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.0$') {
        throw 'Store version must be Major.Minor.Build.0 with no leading zeroes.'
    }
    $parts = $Version.Split('.') | ForEach-Object { [long]$_ }
    if ($parts[0] -lt 1 -or @($parts | Where-Object { $_ -gt 65535 }).Count) {
        throw 'Store version components must be 0..65535 and Major must be positive.'
    }
}
$Version
