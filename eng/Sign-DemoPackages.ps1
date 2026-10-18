param([switch]$Unsigned)
$ErrorActionPreference = 'Stop'

$root = Split-Path $PSScriptRoot -Parent
$sideloadDir = Join-Path $root 'artifacts/sideload'
$certificateDir = Join-Path $root 'artifacts/certificate'
$privateDir = Join-Path $env:RUNNER_TEMP 'liquidglass-demo-signing'
$certificates = [Security.Cryptography.X509Certificates.X509Certificate2Collection]::new()
New-Item -ItemType Directory -Force $sideloadDir | Out-Null

try {
    if (!$Unsigned) {
        if (!$env:PFX_CERT_BASE64 -or !$env:PFX_PASSWORD) {
            throw 'PFX_CERT_BASE64 and PFX_PASSWORD are required for organization signing.'
        }
        New-Item -ItemType Directory -Force $privateDir, $certificateDir | Out-Null
        $pfx = Join-Path $privateDir 'organization.pfx'
        [IO.File]::WriteAllBytes($pfx, [Convert]::FromBase64String($env:PFX_CERT_BASE64))
        $certificates.Import($pfx, $env:PFX_PASSWORD,
            [Security.Cryptography.X509Certificates.X509KeyStorageFlags]::EphemeralKeySet)
        $signingCerts = @($certificates | Where-Object { $_.HasPrivateKey })
        if ($signingCerts.Count -ne 1) { throw 'The PFX must contain exactly one signing certificate with its private key.' }
        $certificate = $signingCerts[0]
        if ($certificate.NotBefore -gt (Get-Date) -or $certificate.NotAfter -le (Get-Date)) {
            throw 'The organization signing certificate is not currently valid.'
        }
        $cer = Join-Path $certificateDir 'LiquidGlassDemo.cer'
        [IO.File]::WriteAllBytes($cer, $certificate.Export(
            [Security.Cryptography.X509Certificates.X509ContentType]::Cert))
    }

    foreach ($platform in @('x64', 'ARM64')) {
        $pattern = "^WUILiquidGlassDemo\.Hlsl_.*_${platform}\.msix$"
        $source = @(Get-ChildItem (Join-Path $root "artifacts/unsigned/$platform") -Recurse -File -Filter '*.msix' |
            Where-Object { $_.Name -match $pattern })
        if ($source.Count -ne 1) { throw "Expected one unsigned $platform Demo package." }
        $targetDir = Join-Path $sideloadDir $platform
        New-Item -ItemType Directory -Force $targetDir | Out-Null
        $target = Join-Path $targetDir $source[0].Name

        if ($Unsigned) {
            Copy-Item $source[0].FullName $target
            continue
        }

        # The Store package keeps its Partner Center identity. Adapt only the
        # sideload copy, since MSIX requires Publisher to match the certificate.
        $unpacked = Join-Path $privateDir $platform
        & $env:MAKEAPPX unpack /o /p $source[0].FullName /d $unpacked
        if ($LASTEXITCODE) { throw "MakeAppx unpack failed for $platform." }
        $manifestPath = Join-Path $unpacked 'AppxManifest.xml'
        [xml]$manifest = Get-Content $manifestPath -Raw
        if ($manifest.Package.Identity.Publisher -ne $certificate.Subject) {
            $manifest.Package.Identity.Publisher = $certificate.Subject
            $manifest.Save($manifestPath)
            & $env:MAKEAPPX pack /o /h SHA256 /d $unpacked /p $target
            if ($LASTEXITCODE) { throw "MakeAppx pack failed for $platform." }
        } else {
            Copy-Item $source[0].FullName $target
        }
        & $env:SIGNTOOL sign /fd SHA256 /f $pfx /p $env:PFX_PASSWORD /sha1 $certificate.Thumbprint $target
        if ($LASTEXITCODE) { throw "SignTool failed for $platform." }

        # Verify cryptographic integrity and the actual signer. Do not require
        # the ephemeral runner to trust the organization's self-signed root.
        $signature = Get-AuthenticodeSignature $target
        if ($signature.SignerCertificate.Thumbprint -ne $certificate.Thumbprint -or
            $signature.Status -notin @('Valid', 'NotTrusted')) {
            throw "Unexpected signature on $platform package: $($signature.Status)"
        }
        Write-Host "Signed $platform Demo with organization certificate $($certificate.Thumbprint)."
    }

    $instructions = if ($Unsigned) {
        'UNSIGNED PR BUILD: organization secrets are unavailable. These MSIX files must be signed with a matching, trusted code-signing certificate before installation. No temporary certificate was generated.'
    } else {
        @"
Signed with the organization certificate: $($certificate.Subject)
Certificate SHA-1 thumbprint: $($certificate.Thumbprint)
LiquidGlassDemo.cer is the public certificate exported from that PFX. It contains no private key.
If Windows does not already trust this certificate, import it into Local Computer > Trusted People before installing the MSIX for your architecture.
The Store .msixupload is for Partner Center submission; it is not the sideload installer.
"@
    }
    $instructions | Set-Content (Join-Path $sideloadDir 'INSTALL.txt') -Encoding utf8
} finally {
    foreach ($item in $certificates) { $item.Dispose() }
    Remove-Item $privateDir -Recurse -Force -ErrorAction SilentlyContinue
}
