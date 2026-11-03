# Composition adapter binary audit

This is a static binary audit, not a runtime compatibility certification. No
build, Demo execution, or rendering tests were performed for this change.

## Available package inventory

The resolver patterns were scanned against the actual x64, x86, and ARM64
`wuceffectsi.dll` files in each of these InteractiveExperiences packages:

| Package | x64 lookup | x86 lookup | ARM64 lookup |
| --- | --- | --- | --- |
| 1.8.250906004 | standalone | ESP-checked standalone | PAC standalone |
| 1.8.260125001 | standalone | ESP-checked standalone | PAC standalone |
| 2.1.10-experimental | standalone | standalone | PAC standalone |
| 2.6.4 | three inline loops | indexed standalone | non-PAC standalone |

Every file has a unique applicable lookup signature (three distinct signatures
for 2.6.4 x64) and a matching DirectPropertyUpdater copy-function signature.
The no-hotpatch x86 copy pattern can also match two bytes into a hotpatch
function; the resolver intentionally prefers the complete hotpatch form first.
This inventory does not establish compatibility with uninspected releases,
including 1.6, or with ARM64EC.

Run the read-only inventory without loading any DLL or executing any hook:

```powershell
python scripts/audit-runtime-patterns.py C:\Users\Kanata\.nuget\packages\microsoft.windowsappsdk.interactiveexperiences
```

The numbers in its output identify `Unique({...})` expressions in source order;
addresses are RVAs. The inventory reads patterns from the implementation.

## 2.6.4 public symbols

Microsoft's symbol server supplied matching public PDBs; SymChk reported
`PDB Matched: TRUE` for GUID/age pairs:

| Architecture | wuceffectsi.pdb GUID | Age |
| --- | --- | --- |
| x64 | 43AA75AF-565C-456C-A984-DA121CED3C99 | 1 |
| x86 | 6B5F62C8-F61B-4FE1-A532-E115CB6AAE55 | 1 |
| ARM64 | 268B899C-D663-4DA3-9BEF-8E146E191FF9 | 1 |

These are public symbols, without private type information or source lines.
Layouts are therefore checked against disassembly, not inferred from PDB types.

## Adaptation

On x64, `FromGuid` is inlined into the Traverser constructor,
`Traverser::EnumerateEffectSubgraphs`, and the GUID-taking EffectNode
constructor. All three loops reference the same 31-entry table. Replacing only
a standalone entrypoint cannot intercept these calls. The new path requires
all three complete loop patterns and a shared table before installing patches.
It preserves each loop's result register and the EffectNode failure branch.

On x86, the GUID is still passed in ECX in the observed machine code. The new
standalone lookup has an indexed table operand and no hotpatch prefix. Retail
1.8 instead saves ESP across the CFG call; that older form is now recognized
as well. The existing fastcall detour remains applicable to these call sites.

On ARM64, 2.6.4 retains a standalone lookup but drops the PAC prologue and adds
X23 to its saved-register set. ADRP is decoded relative to the instruction's
own page, including when the entry and ADRP straddle a page boundary.

`CompiledEffect::GetSubgraphShaderLinkingBody` in 2.6.4 x86 and ARM64 retains
the modeled sizes: CompiledSubgraph is 72/136 bytes; ShaderLinkingBody is
28/48 bytes. The argument, bytecode, function name, constant-buffer and
profile fields retain their existing offsets. DirectPropertyUpdater retains
the source-offset and float-count ABI in all three architectures.

## Remaining verification boundary

Signature matches and these ABI checks do not prove successful rendering,
hook installation under all process conditions, or compatibility of every
graph/property path. Runtime verification remains outstanding by request.
Unknown instruction layouts fail resolution instead of falling back to fixed
RVAs or guessing patch locations.
