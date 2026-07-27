#!/usr/bin/env bash
set -euo pipefail

readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
readonly INSERTER="${SCRIPT_DIR}/macho_add_load_dylib.py"

readonly LOADER_NAME="roblox_modloader.dylib"
readonly LOAD_PATH="@executable_path/${LOADER_NAME}"
readonly BACKUP_SUFFIX=".rml-backup"
readonly DISABLE_LIBRARY_VALIDATION="com.apple.security.cs.disable-library-validation"
readonly GET_TASK_ALLOW="com.apple.security.get-task-allow"
readonly DISABLE_PAGE_PROTECTION="com.apple.security.cs.disable-executable-page-protection"

STUDIO_APP="/Applications/RobloxStudio.app"
DYLIB=""
RESTORE=0
DEBUG=0

die() { printf 'error: %s\n' "$*" >&2; exit 1; }
info() { printf '\033[1;34m==>\033[0m %s\n' "$*"; }
ok() { printf '\033[1;32m  ok\033[0m %s\n' "$*"; }

parse_args() {
  while [ "$#" -gt 0 ]; do
    case "$1" in
      --studio) STUDIO_APP="${2:?--studio needs a path}"; shift 2 ;;
      --dylib) DYLIB="${2:?--dylib needs a path}"; shift 2 ;;
      --debug) DEBUG=1; shift ;;
      --restore) RESTORE=1; shift ;;
      -h|--help) sed -n '2,30p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 0 ;;
      *) die "unknown argument: $1" ;;
    esac
  done
}

find_built_dylib() {
  find "${REPO_ROOT}/build" -type f -name '*roblox_modloader*.dylib' -print0 2>/dev/null \
    | xargs -0 ls -t 2>/dev/null \
    | head -n1
}

main_binary() {
  local plist="${STUDIO_APP}/Contents/Info.plist"
  [ -f "$plist" ] || die "no Info.plist in ${STUDIO_APP} — is that a Studio bundle?"
  local exe
  exe="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleExecutable' "$plist" 2>/dev/null)" \
    || die "Info.plist has no CFBundleExecutable"
  printf '%s/Contents/MacOS/%s' "${STUDIO_APP}" "$exe"
}

plist_set_true() {
  /usr/libexec/PlistBuddy -c "Add :${2} bool true" "$1" 2>/dev/null \
    || /usr/libexec/PlistBuddy -c "Set :${2} true" "$1"
}

resign() {
  local mode="$1" src_binary="$2"
  local entitlements; entitlements="$(mktemp -t rml-entitlements).plist"

  if ! codesign -d --entitlements :- --xml "$src_binary" >"$entitlements" 2>/dev/null \
     || [ ! -s "$entitlements" ]; then
    printf '<?xml version="1.0" encoding="UTF-8"?><!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd"><plist version="1.0"><dict/></plist>' >"$entitlements"
  fi

  if [ "$mode" = "unlock" ]; then
    plist_set_true "$entitlements" "$DISABLE_LIBRARY_VALIDATION"
    plist_set_true "$entitlements" "$DISABLE_PAGE_PROTECTION"
    [ "$DEBUG" -eq 1 ] && plist_set_true "$entitlements" "$GET_TASK_ALLOW"
  fi

  codesign --force --sign - --options runtime --deep \
    --entitlements "$entitlements" "$STUDIO_APP" \
    || { rm -f "$entitlements"; die "codesign failed to sign ${STUDIO_APP}"; }
  rm -f "$entitlements"
}

do_restore() {
  local binary backup
  binary="$(main_binary)"
  backup="${binary}${BACKUP_SUFFIX}"

  [ -f "$backup" ] || die "no backup at ${backup} — nothing to restore"

  info "Restoring the pristine Studio binary"
  mv -f "$backup" "$binary"
  rm -f "${STUDIO_APP}/Contents/MacOS/${LOADER_NAME}"
  ok "binary restored, dylib removed"

  info "Re-signing the restored bundle (ad-hoc)"
  resign "lock" "$binary"
  ok "signed"

  codesign -v "$STUDIO_APP" 2>/dev/null && ok "signature valid" || true
  info "Done. Studio is back to a clean, injection-free state."
}

do_inject() {
  [ -n "$DYLIB" ] || DYLIB="$(find_built_dylib || true)"
  [ -n "$DYLIB" ] || die "no built dylib found under ${REPO_ROOT}/build — build first or pass --dylib"
  [ -f "$DYLIB" ] || die "dylib not found: ${DYLIB}"

  local binary backup dest
  binary="$(main_binary)"
  [ -f "$binary" ] || die "Studio binary not found: ${binary}"
  backup="${binary}${BACKUP_SUFFIX}"
  dest="${STUDIO_APP}/Contents/MacOS/${LOADER_NAME}"

  info "Studio:  ${STUDIO_APP}"
  info "Loader:  ${DYLIB}"

  if [ ! -f "$backup" ]; then
    cp -p "$binary" "$backup"
    ok "backed up pristine binary -> ${backup##*/}"
  else
    ok "backup already exists (keeping the original pristine copy)"
  fi

  cp -f "$DYLIB" "$dest"
  codesign --force --sign - "$dest"
  ok "installed ${LOADER_NAME} into the bundle"

  if /usr/bin/python3 "$INSERTER" has "$binary" "$LOAD_PATH"; then
    ok "load command already present"
  else
    /usr/bin/python3 "$INSERTER" insert "$binary" "$LOAD_PATH" \
      || die "failed to insert the load command"
    ok "added LC_LOAD_DYLIB ${LOAD_PATH}"
  fi

  info "Re-signing the bundle (hardened runtime + disable-library-validation)"
  resign "unlock" "$backup"
  ok "signed"

  info "Verifying"
  codesign -v "$STUDIO_APP" 2>/dev/null && ok "signature valid" || die "signature verification failed"
  if otool -L "$binary" | grep -q "${LOADER_NAME}"; then
    ok "otool confirms the loader is a dependency"
  else
    die "the loader is not listed as a dependency"
  fi
  if [ "$DEBUG" -eq 1 ]; then
    codesign -d --entitlements :- --xml "$binary" 2>/dev/null | grep -q "$GET_TASK_ALLOW" \
      && ok "debuggable (get-task-allow present)" \
      || die "get-task-allow was not applied"
  fi

  info "Done."
}

main() {
  parse_args "$@"
  [ "$(uname -s)" = "Darwin" ] || die "this script is macOS-only"
  [ -f "$INSERTER" ] || die "missing helper: ${INSERTER}"
  STUDIO_APP="${STUDIO_APP%/}"
  [ -d "$STUDIO_APP" ] || die "Studio bundle not found: ${STUDIO_APP}"

  if [ "$RESTORE" -eq 1 ]; then
    do_restore
  else
    do_inject
  fi
}

main "$@"
