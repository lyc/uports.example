#!/bin/sh
# Shared by the distribution's shell validators and standalone project checks.

fail()
{
	printf '  ERR     %s\n' "$*" >&2
	exit 1
}

validation_environment()
{
	# Re-enter once with a clean host environment, including for direct use.
	if [ "${TOOLCHAIN_VALIDATOR_ENV:-}" != clean ]; then
		exec env -i PATH=/usr/bin:/bin LC_ALL=C \
			TOOLCHAIN_VALIDATOR_ENV=clean sh "$0" "$@"
	fi
}

validation_init()
{
	validation_environment "$@"
	profile=$(CDPATH= cd -- "$1" && pwd -P)
	checks=$(readlink -m -- "$2")
	case "$checks" in
		"$profile"|"$profile"/*)
			fail 'validation output must be outside distribution' ;;
	esac
	mkdir -p -- "$checks"
	checks=$(CDPATH= cd -- "$checks" && pwd -P)
	validation_dir=$profile/share/toolchain/validation
	. "$validation_dir/profile.conf"
	HOME=$checks
	TERMINFO=$profile/share/terminfo
	export HOME TERMINFO
	# Keep tutorial output visible through redirections and command capture.
	if [ "${TOOLCHAIN_TUTORIAL_FD:-}" != 3 ]; then
		exec 3>&1
		TOOLCHAIN_TUTORIAL_FD=3
		export TOOLCHAIN_TUTORIAL_FD
	fi
}

shell_quote()
{
	printf "'"
	printf '%s' "$1" | sed "s/'/'\\\\''/g"
	printf "'"
}

command_line()
{
	printf '(cd '
	shell_quote "$(pwd -P)"
	printf ' &&'
	for argument; do
		printf ' '
		shell_quote "$argument"
	done
	printf ')'
}

trace_command()
{
	command_text=$(command_line "$@")
	printf '+ %s\n' "$command_text" >&3
	printf '+ %s\n' "$command_text" >> "$checks/commands.log"
}

show_commands()
{
	printf '\n# %s\n' "$1" >&3
	cat "$2" >&3
}

start_background()
{
	background_log=$1
	shift
	command_text="$(command_line "$@") > $(shell_quote "$background_log")"
	command_text="$command_text 2>&1 &"
	printf '+ %s\n' "$command_text" >&3
	printf '+ %s\n' "$command_text" >> "$checks/commands.log"
	"$@" > "$background_log" 2>&1 &
	background_pid=$!
}

run()
{
	trace_command "$@"
	if "$@"; then status=0; else status=$?; fi
	printf 'exit=%s\n' "$status" >> "$checks/commands.log"
	return "$status"
}
