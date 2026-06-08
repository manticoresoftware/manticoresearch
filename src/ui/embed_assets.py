#!/usr/bin/env python3
"""Convert a dist/ directory of static assets into a C++ source file with embedded byte arrays."""

import os
import sys

MIME_MAP = {
    '.html': 'text/html',
    '.js': 'application/javascript',
    '.css': 'text/css',
    '.svg': 'image/svg+xml',
    '.ico': 'image/x-icon',
    '.png': 'image/png',
    '.json': 'application/json',
    '.woff2': 'font/woff2',
    '.woff': 'font/woff',
    '.txt': 'text/plain',
}

def sanitize(name):
    return ''.join(c if c.isalnum() else '_' for c in name)

def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <dist_dir> <output.cpp>", file=sys.stderr)
        sys.exit(1)

    dist_dir = sys.argv[1]
    output_file = sys.argv[2]

    assets = []
    for root, _, files in os.walk(dist_dir):
        for fname in sorted(files):
            full = os.path.join(root, fname)
            rel = os.path.relpath(full, dist_dir)
            with open(full, 'rb') as f:
                data = f.read()
            ext = os.path.splitext(fname)[1].lower()
            mime = MIME_MAP.get(ext, 'application/octet-stream')
            assets.append((rel, data, mime))

    with open(output_file, 'w') as out:
        out.write('// Auto-generated file - do not edit\n')
        out.write('#include "searchdui.h"\n\n')
        out.write('#if WITH_UI\n\n')

        for rel, data, mime in assets:
            safe = sanitize(rel)
            out.write(f'static const unsigned char kAsset_{safe}[] = {{\n\t')
            for i, b in enumerate(data):
                if i > 0:
                    out.write(',')
                if i > 0 and i % 20 == 0:
                    out.write('\n\t')
                out.write(f'0x{b:02x}')
            out.write('\n};\n\n')

        out.write('const EmbeddedAsset_t g_dUIAssets[] = {\n')
        for rel, data, mime in assets:
            safe = sanitize(rel)
            out.write(f'\t{{ "{rel}", kAsset_{safe}, {len(data)}, "{mime}" }},\n')
        out.write('};\n\n')
        out.write(f'const int g_iUIAssetCount = {len(assets)};\n\n')
        out.write('#endif // WITH_UI\n')

    print(f"Embedded {len(assets)} assets into {output_file}")

if __name__ == '__main__':
    main()
