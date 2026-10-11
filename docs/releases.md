# CI and release

- **CI** builds native x86/x64/ARM64 assets, managed projections, NuGets and the
  C++/C# package consumers. It also runs the native shader reflection validator.
  It never publishes to NuGet.org or creates a Store upload automatically.
- **Debug Core** retains Debug/x64 compilation checks.
- **Demo Preview Package** retains signed x64/ARM64 sideload MSIX and its public
  certificate, using the existing automatic preview version scheme.
- **Release** (`release-nuget.yml`) is one manual entry. Enter the NuGet version;
  the Store version is generated as `YYYY.MMDD.RUN.0`. Both outputs use the exact
  selected source revision. After validation, it publishes both NuGets and creates
  one GitHub Release containing the NuGets and unsigned Store `.msixupload`.

NuGet versions use `Major.Minor.Patch` with an optional prerelease suffix. Normal
CI uses `0.0.0-ci.<run>.<attempt>` for test artifacts. The shared Demo packaging
workflow is internal reuse, not another release entry. Store signing/submission
remains the responsibility of Partner Center; sideload previews keep their own
temporary certificate and ship only its public `.cer`.

GitHub's existing automatic dependency submission remains enabled and already
passes. No extra Windows dependency-submission or workflow-validation workflow
is needed. The projection restore fixes declare the Windows target/RIDs, separate
NuGet intermediates, and exclude the whole shared `obj`/`bin` trees from default
C# source globs.

NuGet trusted publishing must allow the `release-nuget.yml` filename for both
packages (the original automatic publisher used `ci.yml`). Keep `NUGET_USER` and
the existing repository/owner policy; publication uses OIDC.
