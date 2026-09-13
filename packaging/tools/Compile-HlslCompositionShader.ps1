param(
    [Parameter(Mandatory = $true)][string]$InputPath,
    [Parameter(Mandatory = $true)][string]$OutputPath,
    [Parameter(Mandatory = $true)][ValidateSet('Auto', 'Color', 'Sampler', 'MaterializedSampler')][string]$Kind,
    [ValidateSet('Level91', 'Level93', 'Pixel40')][string]$Profile = 'Pixel40',
    [string]$IncludeDirectories = '',
    [string]$Defines = '',
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
        foreach ($hostArch in @('x64', 'x86', 'arm64')) {
            $candidate = Join-Path $version.FullName "$hostArch\fxc.exe"
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

function Get-KindValue([string]$EffectKind) {
    switch ($EffectKind) {
        'Color' { return 0 }
        'Sampler' { return 1 }
        'MaterializedSampler' { return 2 }
        default { throw "Unsupported concrete effect kind '$EffectKind'." }
    }
}

function Get-ProfileValue([string]$ShaderProfile) {
    switch ($ShaderProfile) {
        'Level91' { return 0 }
        'Level93' { return 1 }
        'Pixel40' { return 2 }
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

function Make-GeneratedHeaderSelfContained([string]$Path) {
    $text = [IO.File]::ReadAllText($Path)
    $normalized = [Text.RegularExpressions.Regex]::Replace(
        $text,
        '(?m)^(\s*)const\s+BYTE\s+([A-Za-z_][A-Za-z0-9_]*)\s*\[\]\s*=',
        '$1const unsigned char $2[] =')

    if ($normalized -eq $text) {
        throw "FXC generated header '$Path' did not contain the expected const BYTE shader array declaration."
    }

    [IO.File]::WriteAllText($Path, $normalized, [Text.UTF8Encoding]::new($false))
}

function Write-PreparedShader(
    [string]$ResolvedKind,
    [string]$PreparedPath,
    [string]$DisplayPath,
    [string]$UserSource,
    [string]$ShaderProfile) {
    $builder = [Text.StringBuilder]::new()
    if ($ResolvedKind -ne 'Color') {
        [void]$builder.AppendLine('Texture2D texture0; SamplerState sampler0;')
    }
    [void]$builder.AppendLine("#line 1 `"$DisplayPath`"")
    [void]$builder.Append($UserSource)
    if (!$UserSource.EndsWith("`n")) {
        [void]$builder.AppendLine()
    }

    if ($ResolvedKind -eq 'MaterializedSampler') {
        [void]$builder.AppendLine('#line 1 "WinUI.Composition.Hlsl.Generated.hlsl"')
        [void]$builder.AppendLine('export float4 MaterializeColor(float4 color){return color;}')
        $suffixes = @('', 'CC', 'CW', 'CM', 'WC', 'WW', 'WM', 'MC', 'MW', 'MM', 'C', 'W', 'M')
        foreach ($suffix in $suffixes) {
            [void]$builder.AppendLine('#line 1 "WinUI.Composition.Hlsl.Generated.hlsl"')
            [void]$builder.AppendLine("export float4 PSBody$suffix(float2 uv,float4 samplerDataExt,float4 samplerData){return Shade(uv,samplerDataExt,samplerData);}")
        }
    }
    elseif ($ResolvedKind -eq 'Sampler') {
        $suffixes = @('', 'CC', 'CW', 'CM', 'WC', 'WW', 'WM', 'MC', 'MW', 'MM', 'C', 'W', 'M')
        foreach ($suffix in $suffixes) {
            [void]$builder.AppendLine('#line 1 "WinUI.Composition.Hlsl.Generated.hlsl"')
            [void]$builder.AppendLine("export float4 PSBody$suffix(float2 uv,float4 samplerDataExt){return Shade(uv,samplerDataExt);}")
        }
    }
    else {
        # Validate the Color contract during compilation instead of waiting for the
        # generated library to reach HlslShaderLibrary reflection at runtime.
        [void]$builder.AppendLine('#line 1 "WinUI.Composition.Hlsl.Generated.hlsl"')
        [void]$builder.AppendLine('export float4 __WinUICompositionHlslValidateColor(float4 color){return PSBody(color);}')
    }

    $kindValue = Get-KindValue $ResolvedKind
    $profileValue = Get-ProfileValue $ShaderProfile
    [void]$builder.AppendLine('#line 1 "WinUI.Composition.Hlsl.Metadata.hlsl"')
    [void]$builder.AppendLine("export float4 __WinUICompositionHlsl_Metadata_K${kindValue}_P${profileValue}(float4 value){return value;}")

    [IO.File]::WriteAllText($PreparedPath, $builder.ToString(), [Text.UTF8Encoding]::new($false))
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
$outputBaseName = [IO.Path]::GetFileNameWithoutExtension($outputFull)
$prepared = [IO.Path]::Combine($outputDirectory, $outputBaseName + '.prepared.hlsl')
$displayPath = $inputFull.Replace('\', '/')
$fxc = Resolve-Fxc

$commonArguments = @(
    '/nologo',
    '/Ges',
    '/O3',
    '/WX',
    '/T', (Get-Target $Profile),
    '/I', [IO.Path]::GetDirectoryName($inputFull)
)

if ($IncludeDirectories) {
    foreach ($directory in $IncludeDirectories.Split(';', [StringSplitOptions]::RemoveEmptyEntries)) {
        $trimmed = $directory.Trim()
        if (!$trimmed) { continue }
        $resolvedDirectory = [IO.Path]::GetFullPath($trimmed)
        if (!(Test-Path $resolvedDirectory -PathType Container)) {
            throw "HLSL include directory does not exist: '$resolvedDirectory'."
        }
        $commonArguments += @('/I', $resolvedDirectory)
    }
}

if ($Defines) {
    foreach ($definition in $Defines.Split(';', [StringSplitOptions]::RemoveEmptyEntries)) {
        $trimmed = $definition.Trim()
        if (!$trimmed) { continue }
        $separator = $trimmed.IndexOf('=')
        $name = if ($separator -ge 0) { $trimmed.Substring(0, $separator) } else { $trimmed }
        if ($name -notmatch '^[A-Za-z_][A-Za-z0-9_]*$') {
            throw "Invalid HLSL preprocessor definition '$trimmed'. Expected NAME or NAME=VALUE."
        }
        $commonArguments += @('/D', $trimmed)
    }
}

$resolvedKind = $Kind
if ($Kind -eq 'Auto') {
    $matches = @()
    foreach ($candidate in @('Color', 'Sampler', 'MaterializedSampler')) {
        $probePrepared = [IO.Path]::Combine($outputDirectory, "$outputBaseName.probe.$candidate.hlsl")
        $probeOutput = [IO.Path]::Combine($outputDirectory, "$outputBaseName.probe.$candidate.dxbc")
        try {
            Write-PreparedShader $candidate $probePrepared $displayPath $source $Profile
            $probeArguments = $commonArguments + @('/Fo', $probeOutput, $probePrepared)

            # A failed probe means only that this public contract does not match the
            # user's source. Windows PowerShell 5.1 turns native stderr into a
            # NativeCommandError when ErrorActionPreference=Stop, so temporarily
            # suppress native probe diagnostics and judge the candidate solely by
            # FXC's exit code. The final selected compile still runs under Stop and
            # reports its diagnostics normally.
            $probeExitCode = 1
            $previousErrorActionPreference = $ErrorActionPreference
            try {
                $ErrorActionPreference = 'SilentlyContinue'
                & $fxc @probeArguments *> $null
                $probeExitCode = $LASTEXITCODE
            }
            finally {
                $ErrorActionPreference = $previousErrorActionPreference
            }

            if ($probeExitCode -eq 0) {
                $matches += $candidate
            }
        }
        finally {
            Remove-Item $probePrepared -Force -ErrorAction SilentlyContinue
            Remove-Item $probeOutput -Force -ErrorAction SilentlyContinue
        }
    }

    if ($matches.Count -eq 0) {
        throw "Could not infer the Composition HLSL kind for '$inputFull'. The shader must match exactly one Color, Sampler, or MaterializedSampler contract, or set <Kind> explicitly."
    }
    if ($matches.Count -ne 1) {
        throw "The Composition HLSL kind for '$inputFull' is ambiguous ($($matches -join ', ')). Set <Kind> explicitly."
    }
    $resolvedKind = $matches[0]
    Write-Host "Inferred Composition HLSL kind: $resolvedKind"
}

Write-PreparedShader $resolvedKind $prepared $displayPath $source $Profile
$arguments = $commonArguments + @('/Fo', $outputFull)

$headerFull = ''
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

if ($headerFull) {
    Make-GeneratedHeaderSelfContained $headerFull
}

Write-Host "Compiled Composition HLSL: $inputFull -> $outputFull ($resolvedKind/$Profile)"
