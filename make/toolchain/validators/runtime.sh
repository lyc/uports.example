#!/bin/sh
set -eu
. "$(dirname -- "$0")/common.sh"
validation_init "$@"

index=0
while IFS= read -r relative; do
	index=$((index + 1))
	log=$checks/runtime-$index.log
	run timeout 15 ldd "$profile/$relative" > "$log" 2>&1
	if grep -q 'not found' "$log"; then
		fail "unresolved runtime dependency: $relative; see $log"
	fi
	# Keep spaces in paths; the final parenthesized field is ldd's address.
	sed -n 's/^[[:space:]]*\([^ ]*\) => \(.*\) (0x[^)]*).*$/\1|\2/p' \
		"$log" > "$log.paths"
	while IFS='|' read -r name library; do
		case "$name" in
			libc.so.*|libm.so.*|libpthread.so.*|libdl.so.*|\
			librt.so.*|libresolv.so.*|libutil.so.*|\
			libanl.so.*|ld-linux*.so.*)
				continue ;;
		esac
		library=$(readlink -f -- "$library")
		case "$library" in
			"$profile"/lib/runtime/*) ;;
			*) fail "external runtime library: $library" ;;
		esac
	done < "$log.paths"
done < "$validation_dir/host-elf.list"
printf '  CHECK   private host runtime libraries (%s ELF files)\n' "$index"
