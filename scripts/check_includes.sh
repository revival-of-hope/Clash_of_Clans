#!/usr/bin/env bash
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root_dir"

# Only scan C/C++ source files
glob='*.{h,hpp,hh,c,cc,cpp,cxx,mm,mpp}'

fail() {
  echo "ERROR: Non-canonical Contract include paths found." >&2
  echo "Use: #include \"Classes/Contract/...\"" >&2
  echo "" >&2
  echo "$1" >&2
  exit 1
}

# 1) Old style: #include "Contract/..."
hits=$(rg -n -S --glob "$glob" '#include\s+"Contract/' Classes || true)
if [[ -n "$hits" ]]; then
  fail "$hits"
fi

# 2) Shortcut styles enabled by include roots (common drift):
#    #include "Engine/...", "Gameplay/...", "Integration/..."
# These are subtrees of Classes/Contract, so they must be included via Classes/Contract/...
hits=$(rg -n -S --glob "$glob" '#include\s+"(Engine|Gameplay|Integration)/' Classes || true)
if [[ -n "$hits" ]]; then
  fail "$hits"
fi

# (Optional) 3) Angle-bracket variants if anyone uses them:
hits=$(rg -n -S --glob "$glob" '#include\s+<(Contract|Engine|Gameplay|Integration)/' Classes || true)
if [[ -n "$hits" ]]; then
  fail "$hits"
fi

echo "OK: canonical Contract include paths"
exit 0
