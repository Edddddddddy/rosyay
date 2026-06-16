#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

failed=0

fail() {
  echo "::error::$1"
  failed=1
}

info() {
  echo "== $1 =="
}

info "Checking generated ROS2 directories"
generated_dirs="$(find ros2-src -type d \( -name build -o -name install -o -name log -o -name .colcon \) -prune -print)"
if [[ -n "$generated_dirs" ]]; then
  echo "$generated_dirs"
  fail "Generated ROS2 build directories must not be committed under ros2-src."
fi

info "Checking ROS2 package structure"
shopt -s nullglob
packages=(ros2-src/*)
if [[ ${#packages[@]} -eq 0 ]]; then
  fail "No packages found under ros2-src."
fi

for pkg in "${packages[@]}"; do
  [[ -d "$pkg" ]] || continue
  pkg_name="$(basename "$pkg")"

  [[ -f "$pkg/CMakeLists.txt" ]] || fail "$pkg_name is missing CMakeLists.txt."
  [[ -f "$pkg/package.xml" ]] || fail "$pkg_name is missing package.xml."
  [[ -f "$pkg/README.md" ]] || fail "$pkg_name is missing README.md."
  [[ -d "$pkg/src" ]] || fail "$pkg_name is missing src/."

  if [[ -f "$pkg/package.xml" ]]; then
    declared_name="$(sed -n 's:.*<name>\(.*\)</name>.*:\1:p' "$pkg/package.xml" | head -n 1)"
    if [[ "$declared_name" != "$pkg_name" ]]; then
      fail "$pkg_name package.xml declares name '$declared_name'."
    fi
  fi
done

info "Checking CRLF line endings"
mapfile -d '' text_files < <(
  find . \
    -path ./.git -prune -o \
    -type f \( \
      -name '*.md' -o \
      -name '*.cpp' -o \
      -name '*.hpp' -o \
      -name '*.h' -o \
      -name '*.py' -o \
      -name '*.xml' -o \
      -name '*.yaml' -o \
      -name '*.yml' -o \
      -name '*.srv' -o \
      -name '*.urdf' -o \
      -name '*.sdf' -o \
      -name '*.rviz' -o \
      -name 'CMakeLists.txt' -o \
      -name '.editorconfig' -o \
      -name '.clang-format' \
    \) -print0
)

if [[ ${#text_files[@]} -gt 0 ]]; then
  crlf_files="$(grep -Il $'\r' "${text_files[@]}" || true)"
  if [[ -n "$crlf_files" ]]; then
    echo "$crlf_files"
    fail "CRLF line endings found. Use LF."
  fi
fi

info "Checking trailing whitespace"
if [[ ${#text_files[@]} -gt 0 ]]; then
  if grep -nH -E '[[:blank:]]$' "${text_files[@]}"; then
    fail "Trailing whitespace found."
  fi
fi

if [[ "$failed" -ne 0 ]]; then
  echo "ros2-src quality check failed."
  exit 1
fi

echo "ros2-src quality check passed."
