# CI and releases

| Workflow | Trigger | Output |
| --- | --- | --- |
| CI | `master` pushes, pull requests, manual runs, reusable release validation | Native x86/x64/ARM64 assets, projections, test NuGets, C++/C# package-consumer validation. No feed publication or Store package. |
| Debug Core | Existing branch push rules or manual run | Debug/x64 shader and core validation. |
| Demo Preview Package | `master` / `fix/liquid-glass-corner-optics` pushes affecting the Demo, or manual run | Signed x64/ARM64 sideload MSIX plus the public preview certificate. No Store upload or GitHub Release. |
| Release NuGet | **Manual only**, required `package_version` | Validates the exact requested version through CI, publishes both packages to NuGet.org, and creates a `nuget-<version>` GitHub Release. |
| Release Microsoft Store Package | **Manual only**, required `package_version` | Unsigned x64/ARM64 `.msixupload` / `.msixbundle` artifacts and a `store-<version>` GitHub Release. |
| Windows Dependency Submission | Manifest pushes, PR restore checks, or manual run | Restores native and managed dependency graphs with Visual Studio MSBuild; supplements GitHub's managed Linux snapshot with native dependency coverage on trusted push/manual runs. |

## Release a version

Select the desired branch/ref in Actions and run the appropriate release workflow.
NuGet accepts `Major.Minor.Patch` or a prerelease suffix, for example `1.2.0-rc.1`.
Store accepts `Major.Minor.Build.0` with components up to 65535 and a positive Major.
Release workflows require an explicit version; a push or tag does not publish anything.
CI uses `0.0.0-ci.<run>.<attempt>` for its local test packages. Sideload previews keep
the existing `YYYY.MMDD.RUN.0` convention when no version is entered.

The Store workflow creates an upload package for Partner Center; it does not submit
to Partner Center automatically. Microsoft Store signs the package when published.
The independent sideload workflow signs its MSIX with a temporary certificate and
ships only the public `.cer`; install that certificate into Trusted People first.

## One-time external configuration

- Update the NuGet.org trusted publishing policy for both packages to allow the
  `release-nuget.yml` workflow filename (previously `ci.yml`). Keep the existing
  repository/owner and `NUGET_USER` Actions secret. Publication uses OIDC.

GitHub's automatic dependency submission can stay enabled: its Linux managed
restore and snapshot submission now pass. That workflow skips native project
restoration on Linux. **Windows Dependency Submission** therefore also restores
the actual Visual C++ dependency graphs and uses a distinct snapshot correlator.
No dependency graph, Dependabot, or repository security setting needs to be disabled.

The managed projects permit Windows reference-pack restoration and the projection
libraries declare `win-x64`, `win-x86`, and `win-arm64` RIDs. Native references map
`x86` to `Win32` and default `AnyCPU` to `x64`. Compilation, XAML generation, and
native dependency restoration use Windows/Visual Studio rather than Linux.
The two projection projects use separate `obj/<project-name>/` directories so
their NuGet asset graphs cannot overwrite each other. The project-based managed
control consumer also accepts transitive `native` build dependencies.
