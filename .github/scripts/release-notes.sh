#!/usr/bin/env bash
# Builds the GitHub Release text for a tag (e.g. v1.1.0): the matching CHANGELOG.md
# section followed by the download table. Usage: release-notes.sh v1.1.0 > notes.md
set -euo pipefail
version="${1#v}"

changes=$(awk -v v="$version" '
  /^## \[/ { if (found) exit; if (index($0, "## [" v "]") == 1) { found = 1; next } }
  found && /^\[[0-9]/ { exit }
  found { print }
' CHANGELOG.md)

if [ -z "$(echo "$changes" | tr -d '[:space:]')" ]; then
  changes="See CHANGELOG.md."
fi

cat <<NOTES
**Fase Groove Piano** – funky piano grooves for soulful, deep & jackin' house, by Fabio 'Pitch' Serra.

## What's new
$changes

## Download

| System | File |
|---|---|
| Windows 10/11 (64-bit) | \`FaseGroovePiano-Windows.zip\` – VST3 + Standalone |
| macOS 10.13+ (Intel & Apple Silicon) | \`FaseGroovePiano-macOS.zip\` – VST3 + AU + Standalone |
| Linux | \`FaseGroovePiano-Linux.zip\` – VST3 + Standalone |

User guide inside the zip: \`GUIDE.md\` (English) · \`GUIDA.md\` (Italiano).
macOS: the plug-in is not notarized – if it is blocked, run \`xattr -cr\` on it (see the guide).
Full history: [CHANGELOG](https://github.com/wsx79/fasegroovepiano/blob/main/CHANGELOG.md) · [Novità in italiano](https://github.com/wsx79/fasegroovepiano/blob/main/CHANGELOG.it.md)

Free & open source (GNU AGPL v3). Support the project: [Ko-fi](https://ko-fi.com/fabios1979) · [PayPal](https://paypal.me/fabios79)
NOTES
