"""Read-only PE signature inventory; does not load DLLs or execute patches.

Usage: python scripts/audit-runtime-patterns.py <InteractiveExperiences package root>
Patterns are read from the implementation so this cannot audit stale copies.
"""
import pathlib
import re
import struct
import sys

source = (pathlib.Path(__file__).resolve().parents[1] /
          'src/WinUI.Composition.Hlsl/Runtime/CustomEffectRuntime.cpp').read_text(encoding='utf-8-sig')
patterns = []
for match in re.finditer(r'Unique\(\{([^}]+)\}', source):
    values = [int(x.strip(), 0) for x in match[1].split(',') if x.strip()]
    patterns.append(re.compile(b''.join(b'.' if x < 0 else re.escape(bytes([x])) for x in values), re.S))

for file in sorted(pathlib.Path(sys.argv[1]).glob('*/runtimes-framework/win-*/native/wuceffectsi.dll')):
    if 'arm64ec' in str(file):
        continue  # ARM64EC is not an advertised adapter architecture.
    data = file.read_bytes()
    pe = struct.unpack_from('<I', data, 60)[0]
    count, opt_size = struct.unpack_from('<H', data, pe + 6)[0], struct.unpack_from('<H', data, pe + 20)[0]
    sections = []
    for i in range(count):
        start = pe + 24 + opt_size + 40 * i
        size, rva, raw_size, offset = struct.unpack_from('<IIII', data, start + 8)
        flags = struct.unpack_from('<I', data, start + 36)[0]
        if flags & 0x60000000 == 0x60000000:
            sections.append((rva, data[offset:offset + min(size, raw_size)]))
    matches = []
    for index, pattern in enumerate(patterns):
        hits = [hex(rva + m.start()) for rva, section in sections for m in pattern.finditer(section)]
        if hits:
            matches.append(f'{index}=' + ','.join(hits))
    print(file.parents[3].name, file.parents[1].name, ' '.join(matches))
