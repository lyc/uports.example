#!/bin/sh
set -eu
. "$(dirname -- "$0")/common.sh"
validation_init "$@"

cc=$profile/bin/$TARGET-gcc
run timeout 60 "$cc" -dumpversion > "$checks/compiler-version.log"
[ "$(cat "$checks/compiler-version.log")" = "$GCC_VERSION" ] ||
	fail "unexpected compiler version; expected $GCC_VERSION"
run timeout 60 "$cc" -dumpmachine > "$checks/compiler-target.log"
[ "$(cat "$checks/compiler-target.log")" = "$TARGET" ] ||
	fail "unexpected compiler target; expected $TARGET"
for flag in -print-prog-name=as -print-prog-name=ld \
	-print-prog-name=cc1 -print-libgcc-file-name; do
	path=$(run timeout 60 "$cc" "$flag")
	resolved=$(readlink -f -- "$path")
	[ -f "$resolved" ] || fail "missing compiler input: $path"
	case "$resolved" in
		"$profile"/*) ;;
		*) fail "compiler resolves outside distribution: $resolved" ;;
	esac
done
printf '  CHECK   compiler identity and internal tool/libgcc paths\n'
