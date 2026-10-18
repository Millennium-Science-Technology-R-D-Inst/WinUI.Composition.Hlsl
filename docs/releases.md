# CI and release

- **CI** runs on pushes to all branches, pull requests to master, and manual
  dispatch (documentation-only pushes are excluded). It builds native
  x86/x64/ARM64 assets, managed projections, NuGets and C#/C++ package consumers,
  and runs the native shader reflection validator. Its final Demo job produces
  x64/ARM64 sideload MSIX and the organization public certificate. Demo packaging
  still runs if a separate Debug or C# check fails, provided NuGet packing passed.
  It never publishes to NuGet.org or creates a Store upload automatically.
- **Debug Core** is an internal reusable job of CI, retaining Debug/x64 checks
  without a second push-triggered workflow. Debug and Release are different
  configurations, so their native compilations are both intentional.
- **Demo Preview Package** is a manual convenience entry into the same CI with
  an optional MSIX version. It has no additional push trigger.
- **Release** (`release-nuget.yml`) is one manual entry. Enter one version for NuGet, Store and sideload packages.
  All outputs use the exact selected source revision. After validation, it publishes both NuGets and creates
  one GitHub Release containing the NuGets, an unsigned Store `.msixupload`,
  signed x64/ARM64 sideload MSIX and public certificate.

The native Release libraries are built once per architecture in **Pack native
packages**. Both the Demo's Debug/x64 compile check and Release/x64/ARM64 MSIX
builds replace source project references with exact references to those NuGets.
The MSIX runtime DLL hashes are checked against the downloaded packages. An
isolated NuGet cache and package source mapping prevent a previously published
package with the same version from replacing this run's binaries. Release asks
the same final Demo job for a Store upload; it does not rebuild the Demo or its
native dependencies in a second Store job.

Versions accept three or four numeric components, including `1.1.1.1`, with an
optional NuGet prerelease suffix. NuGet's own version parser normalizes package
versions (for example, `1.2.0.0` becomes `1.2.0`); MSIX represents the same numeric
version with four components (`1.2.0.0`). Nonzero revisions are preserved in both.
MSIX has no prerelease suffix; its numeric components come from the same input.
See [NuGet version normalization](https://learn.microsoft.com/en-us/nuget/concepts/package-versioning#normalized-version-numbers)
and [MSIX Identity schema](https://learn.microsoft.com/en-us/uwp/schemas/appxpackage/uapmanifestschema/element-identity). Normal
CI uses `0.0.0-ci.<run>.<attempt>` for test artifacts. The shared Demo packaging
workflow is internal reuse, not another release entry. Store signing/submission
remains the responsibility of Partner Center.

## Sideload signing

Make the organization Actions secrets `PFX_CERT_BASE64` (Base64-encoded signing
PFX) and `PFX_PASSWORD` available to this repository. Reusable workflow callers
explicitly pass both secrets through CI to Demo packaging. No temporary
certificate is generated. The private PFX exists only under the runner's
temporary directory during signing and is removed even if signing fails.

Sideload copies use the certificate's Subject as their manifest Publisher when
it differs from the Store identity. Store packages retain the Partner Center
Publisher. A different Publisher produces a different Windows package identity;
old temporary-certificate installations may need to be removed once when
switching to the organization certificate. Future previews use the stable
organization identity.

The sideload artifact includes `LiquidGlassDemo.cer`, exported from the actual
signing PFX without a private key, and `INSTALL.txt`. If the certificate is not
already trusted, install the public certificate under **Local Computer > Trusted
People** before installing the MSIX for the device's architecture. Fork and
Dependabot PRs without secrets produce an explicitly named `-Sideload-Unsigned`
artifact with instructions instead. Missing secrets on ordinary builds fail
with a configuration error rather than silently generating another certificate.

## Store submission

Upload **`WUILiquidGlassDemo.Hlsl_<version>.msixupload`** to Partner Center.
It contains the unsigned `_neutral.msixbundle`, which in turn contains the x64
and ARM64 MSIX packages. These are nested formats of the same submission, not
two separate uploads. Only `.msixupload` is exposed as the Store artifact and
Release asset to avoid ambiguity. `neutral` describes the bundle container;
its application packages are architecture-specific.

Microsoft also accepts a bundle directly, but [recommends the upload
file](https://learn.microsoft.com/en-us/windows/msix/package/packaging-uwp-apps#generate-an-app-package-upload-file-for-store-submission).
This workflow creates the upload file manually from the bundle and currently
omits the optional `.appxsym` crash-analysis symbols. Install the separately
signed sideload MSIX for local testing.

GitHub's existing automatic dependency submission remains enabled and already
passes. No extra Windows dependency-submission or workflow-validation workflow
is needed. The projection restore fixes declare the Windows target/RIDs, separate
NuGet intermediates, and exclude the whole shared `obj`/`bin` trees from default
C# source globs.

NuGet trusted publishing must allow the `release-nuget.yml` filename for both
packages (the original automatic publisher used `ci.yml`). Keep `NUGET_USER` and
the existing repository/owner policy; publication uses OIDC.
