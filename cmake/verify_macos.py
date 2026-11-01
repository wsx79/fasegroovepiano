#!/usr/bin/env python3
"""Fail before signing if a required universal macOS bundle targets another OS."""
import argparse
import plistlib
import re
import subprocess
import sys
from pathlib import Path


def version(value):
    parts = tuple(int(part) for part in value.split('.'))
    return parts + (0,) * (3 - len(parts))


def run(*args):
    return subprocess.check_output(args, text=True).strip()


def verify(bundle):
    with (bundle / 'Contents/Info.plist').open('rb') as stream:
        info = plistlib.load(stream)
    if version(info.get('LSMinimumSystemVersion', '0')) != version('11.0'):
        raise ValueError(f'{bundle}: LSMinimumSystemVersion must be 11.0')
    binary = bundle / 'Contents/MacOS' / info['CFBundleExecutable']
    arches = set(run('xcrun', 'lipo', '-archs', str(binary)).split())
    if arches != {'arm64', 'x86_64'}:
        raise ValueError(f'{binary}: expected arm64 + x86_64, found {sorted(arches)}')
    for arch in sorted(arches):
        output = run('xcrun', 'otool', '-arch', arch, '-l', str(binary))
        # Older Intel targets may use LC_VERSION_MIN_MACOSX instead of LC_BUILD_VERSION.
        commands = re.split(r'Load command \d+', output)
        minima = []
        for command in commands:
            if re.search(r'\bcmd LC_BUILD_VERSION\b', command):
                match = re.search(r'^\s*minos\s+([\d.]+)\s*$', command, re.M)
            elif re.search(r'\bcmd LC_VERSION_MIN_MACOSX\b', command):
                match = re.search(r'^\s*version\s+([\d.]+)\s*$', command, re.M)
            else:
                continue
            if match is None:
                raise ValueError(f'{binary} ({arch}): missing deployment version')
            minima.append(match[1])
        if len(minima) != 1 or version(minima[0]) != version('11.0'):
            raise ValueError(f'{binary} ({arch}): minos={minima}, expected 11.0')
        print(f'{binary} ({arch}): macOS {minima[0]}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('bundles', type=Path, nargs='+', help='Required .app/.vst3/.component bundles')
    args = parser.parse_args()
    failed = False
    for bundle in args.bundles:
        try:
            verify(bundle)
        except (OSError, ValueError, KeyError, plistlib.InvalidFileException, subprocess.CalledProcessError) as error:
            print(f'::error::{error}', file=sys.stderr)
            failed = True
    return int(failed)


if __name__ == '__main__':
    sys.exit(main())
