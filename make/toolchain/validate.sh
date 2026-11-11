#!/bin/sh
# Standalone distribution acceptance; no workspace or Python is required.
set -eu

profile=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
validation_dir=$profile/share/toolchain/validation
. "$validation_dir/common.sh"
validation_environment "$@"

[ "$#" -le 2 ] || fail 'usage: validate.sh [all|check-name] [output-directory]'
mode=${1:-all}
checks=${2:-$(mktemp -d "${TMPDIR:-/tmp}/toolchain-check.XXXXXX")}
if [ -e "$checks" ]; then
	[ -d "$checks" ] && [ -z "$(ls -A "$checks")" ] ||
		fail 'validation output directory must be empty'
fi
validation_init "$profile" "$checks"
printf 'Validation artifacts: %s\n' "$checks"

if [ -f "$profile/SHA256SUMS" ]; then
	(cd "$profile" && run sha256sum -c SHA256SUMS) \
		> "$checks/checksums.log" 2>&1
fi
if [ "$mode" != all ]; then
	grep -Fxq -- "$mode" "$validation_dir/checks.list" ||
		fail "unsupported validation check: $mode"
fi
while IFS= read -r step; do
	[ "$mode" = all ] || [ "$mode" = "$step" ] || continue
	run sh "$validation_dir/$step.sh" "$profile" "$checks"
done < "$validation_dir/checks.list"
printf 'PASS: %s validation\n' "$mode"
