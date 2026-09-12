param(
    [Parameter(Mandatory = $true)][string]$InputPath,
    [Parameter(Mandatory = $true)][string]$OutputPath,
    [Parameter(Mandatory = $true)][ValidateSet('Color', 'Sampler', 'MaterializedSampler')][string]$Kind,
    [ValidateSet('Level91', 'Level93', 'Pixel40')][string]$Profile = 'Pixel40',
    [string]$HeaderPath = '',
    [string]$VariableName = ''
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

function Resolve-Fxc {
    if ($env:HLSL_FXC_PATH -and (Test-Path $env:HLSL_FXC_PATH)) {
        return (Resolve-Path $env:HLSL_FXC_PATH).Path
    }

    $command = Get-Command fxc.exe -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($command) {
        return $command.Source
    }

    $kitsRoot = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10\bin'
    if (!(Test-Path $kitsRoot)) {
        throw 'Windows SDK FXC was not found. Install a Windows 10/11 SDK or set HLSL_FXC_PATH.'
    }

    $versions = Get-ChildItem $kitsRoot -Directory |
        Where-Object { $_.Name -match '^10\.0\.\d+\.\d+$' } |
        Sort-Object { [version]$_.Name } -Descending

    foreach ($version in $versions) {
        foreach ($host in @('x64', 'x86', 'arm64')) {
            $candidate = Join-Path $version.FullName "$host\fxc.exe"
            if (Test-Path $candidate) {
                return $candidate
            }
        }
    }

    throw 'Windows SDK FXC was not found. Install a Windows 10/11 SDK or set HLSL_FXC_PATH.'
}

function Get-Target([string]$ShaderProfile) {
    switch ($ShaderProfile) {
        'Level91' { return 'lib_4_0_level_9_1_ps_only' }
        'Level93' { return 'lib_4_0_level_9_3_ps_only' }
        'Pixel40' { return 'lib_4_0' }
        default { throw "Unsupported shader profile '$ShaderProfile'." }
    }
}

function Get-SafeIdentifier([string]$Name) {
    if (!$Name) { return '' }
    $safe = [Text.RegularExpressions.Regex]::Replace($Name, '[^A-Za-z0-9_]', '_')
    if ($safe -notmatch '^[A-Za-z_]') {
        $safe = "_$safe"
    }
    if ($safe -ne $Name) {
        Write-Host "Sanitized generated HLSL header variable '$Name' -> '$safe'."
    }
    return $safe
}

$inputFull = [IO.Path]::GetFullPath($InputPath)
$outputFull = [IO.Path]::GetFullPath($OutputPath)
if (!(Test-Path $inputFull)) {
    throw "HLSL input '$inputFull' does not exist."
}

$bytes = [IO.File]::ReadAllBytes($inputFull)
if ($bytes.Length -eq 0 -or $bytes.Length -gt 1MB) {
    throw "HLSL source must be non-empty and no larger than 1 MiB: '$inputFull'."
}

$source = [IO.File]::ReadAllText($inputFull)
if ($source.IndexOf([char]0) -ge 0) {
    throw "HLSL source contains an embedded NUL: '$inputFull'."
}

$outputDirectory = [IO.Path]::GetDirectoryName($outputFull)
[IO.Directory]::CreateDirectory($outputDirectory) | Out-Null
$prepared = [IO.Path]::Combine(
    $outputDirectory,
    [IO.Path]::GetFileNameWithoutExtension($outputFull) + '.prepared.hlsl')

$displayPath = $inputFull.Replace('\', '/')
$builder = [Text.StringBuilder]::new()
if ($Kind -ne 'Color') {
    [void]$builder.AppendLine('Texture2D texture0; SamplerState sampler0;')
}
[void]$builder.AppendLine("#line 1 `"$displayPath`"")
[void]$builder.Append($source)
if (!$source.EndsWith("`n")) {
    [void]$builder.AppendLine()
}

if ($Kind -eq 'MaterializedSampler') {
    [void]$builder.AppendLine('#line 1 "WinUI.Composition.Hlsl.Generated.hlsl"')
    [void]$builder.AppendLine('export float4 MaterializeColor(float4 color){return color;}')
    $suffixes = @('', 'CC', 'CW', 'CM', 'WC', 'WW', 'WM', 'MC', 'MW', 'MM', 'C', 'W', 'M')
    foreach ($suffix in $suffixes) {
        [void]$builder.AppendLine('#line 1 "WinUI.Composition.Hlsl.Generated.hlsl"')
        [void]$builder.AppendLine("export float4 PSBody$suffix(float2 uv,float4 samplerDataExt,float4 samplerData){return Shade(uv,samplerDataExt,samplerData);}")
    }
}
elseif ($Kind -eq 'Sampler') {
    $suffixes = @('', 'CC', 'CW', 'CM', 'WC', 'WW', 'WM', 'MC', 'MW', 'MM', 'C', 'W', 'M')
    foreach ($suffix in $suffixes) {
        [void]$builder.AppendLine('#line 1 "WinUI.Composition.Hlsl.Generated.hlsl"')
        [void]$builder.AppendLine("export float4 PSBody$suffix(float2 uv,float4 samplerDataExt){return Shade(uv,samplerDataExt);}")
    }
}
else {
    # Force the public color ABI to resolve at build time. FXC reports a missing or
    # incompatible PSBody(float4) here instead of deferring that error to app startup.
    [void]$builder.AppendLine('#line 1 "WinUI.Composition.Hlsl.Generated.hlsl"')
    [void]$builder.AppendLine('export float4 __WinUICompositionHlslValidateColor(float4 color){return PSBody(color);}')
}

[IO.File]::WriteAllText($prepared, $builder.ToString(), [Text.UTF8Encoding]::new($false))

$fxc = Resolve-Fxc
$arguments = @(
    '/nologo',
    '/Ges',
    '/O3',
    '/WX',
    '/T', (Get-Target $Profile),
    '/Fo', $outputFull
)

if ($HeaderPath) {
    $headerFull = [IO.Path]::GetFullPath($HeaderPath)
    [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($headerFull)) | Out-Null
    $arguments += @('/Fh', $headerFull)
    if ($VariableName) {
        $arguments += @('/Vn', (Get-SafeIdentifier $VariableName))
    }
}
$arguments += $prepared

& $fxc @arguments
if ($LASTEXITCODE -ne 0) {
    throw "FXC failed for '$inputFull' with exit code $LASTEXITCODE."
}

Write-Host "Compiled Composition HLSL: $inputFull -> $outputFull ($Kind/$Profile)"
