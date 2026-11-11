#!/bin/sh
set -eu
. "$(dirname -- "$0")/common.sh"
validation_init "$@"
[ "$TARGET" = i386-jos-elf ] || fail 'JOS validation needs i386-jos-elf'

prefix=$profile/bin/$TARGET-
cc=${prefix}gcc
cat > "$checks/check.c" <<'EOF'
volatile unsigned long long n = 100, d = 3, r;
void _start(void)
{
	r = n / d;
	for (;;) {}
}
EOF
show_commands "Freestanding C probe: $checks/check.c" "$checks/check.c"
run timeout 60 "$cc" -O0 -ffreestanding -fno-stack-protector -gstabs \
	-c "$checks/check.c" -o "$checks/check.o" > "$checks/compile.log" 2>&1
run timeout 60 "$cc" -nostdlib -Wl,-e,_start "$checks/check.o" -lgcc \
	-o "$checks/check.elf" > "$checks/link.log" 2>&1
run "${prefix}readelf" -h "$checks/check.elf" > "$checks/header.log"
grep -q ELF32 "$checks/header.log" || fail 'expected ELF32 output'
grep -q 'Intel 80386' "$checks/header.log" || fail 'expected i386 output'
run "${prefix}nm" -u "$checks/check.elf" > "$checks/undefined.log"
[ ! -s "$checks/undefined.log" ] || fail 'unresolved libgcc symbols'
run "${prefix}objdump" --stabs "$checks/check.o" > "$checks/stabs.log"
grep -q 'FUN.*_start' "$checks/stabs.log" || fail 'missing STABS function'

# Build only a private copy of the source shipped with the distribution.
[ ! -e "$checks/jos" ] || fail 'JOS output directory already exists'
run cp -R "$validation_dir/source" "$checks/jos"
run chmod -R u+w "$checks/jos"
run timeout 180 make -C "$checks/jos" -j2 "GCCPREFIX=$prefix" \
	"QEMU=$profile/bin/qemu-system-i386" UPSTREAM= \
	> "$checks/jos-build.log" 2>&1
for arch in i386 x86_64; do
	run "$profile/bin/qemu-system-$arch" --version \
		> "$checks/qemu-$arch-version.log"
done

qemu_pid=
cleanup()
{
	[ -n "$qemu_pid" ] || return 0
	kill "$qemu_pid" 2>/dev/null || :
	cleanup_tick=0
	while kill -0 "$qemu_pid" 2>/dev/null && [ "$cleanup_tick" -lt 50 ]; do
		sleep .1
		cleanup_tick=$((cleanup_tick + 1))
	done
	kill -KILL "$qemu_pid" 2>/dev/null || :
	wait "$qemu_pid" 2>/dev/null || :
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' HUP TERM

# Select a localhost port and verify the listening socket belongs to our QEMU.
# /proc avoids a Python/netcat/socat prerequisite in the distribution validator.
port=$((20000 + $$ % 30000))
attempt=0
ready=no
while [ "$attempt" -lt 20 ]; do
	start_background "$checks/qemu.log" \
		"$profile/bin/qemu-system-i386" -display none -monitor none \
		-serial "file:$checks/serial.log" -no-reboot -net none \
		-m 32 -S \
		-gdb "tcp:127.0.0.1:$port" \
		-drive \
		"file=$checks/jos/obj/kern/kernel.img,format=raw,snapshot=on"
	qemu_pid=$background_pid
	hex=$(printf '%04X' "$port")
	tick=0
	while kill -0 "$qemu_pid" 2>/dev/null && [ "$tick" -lt 100 ]; do
		inode=$(awk -v address="0100007F:$hex" \
			'$2 == address && $4 == "0A" {print $10; exit}' \
			/proc/net/tcp)
		if [ -n "$inode" ]; then
			for descriptor in /proc/"$qemu_pid"/fd/*; do
				link=$(readlink "$descriptor" 2>/dev/null || :)
				if [ "$link" = "socket:[$inode]" ]; then
					ready=yes
					break
				fi
			done
		fi
		[ "$ready" = yes ] && break
		sleep .1
		tick=$((tick + 1))
	done
	[ "$ready" = yes ] && break
	if kill -0 "$qemu_pid" 2>/dev/null; then
		fail 'QEMU did not open its GDB listener; see qemu.log'
	fi
	cleanup
	qemu_pid=
	port=$((port + 1))
	attempt=$((attempt + 1))
done
[ "$ready" = yes ] || fail 'QEMU did not start its GDB listener; see qemu.log'

cat > "$checks/commands.gdb" <<EOF
set pagination off
set confirm off
set remotetimeout 5
file $checks/jos/obj/kern/kernel
directory $checks/jos
target remote 127.0.0.1:$port
hbreak i386_init
continue
if \$eip != (unsigned int)&i386_init
  quit 1
end
info registers eip esp
info line i386_init
set \$before = \$eip
stepi
if \$eip == \$before
  quit 1
end
echo export-breakpoint-step-ok\n
detach
quit
EOF
show_commands "GDB commands: $checks/commands.gdb" "$checks/commands.gdb"
run timeout 30 "${prefix}gdb" -nx -batch -x "$checks/commands.gdb" \
	> "$checks/gdb.log" 2>&1
grep -q export-breakpoint-step-ok "$checks/gdb.log" || fail 'GDB step failed'
grep -q kern/init.c "$checks/gdb.log" || fail 'GDB source lookup failed'
tick=0
while ! grep -q 'Welcome to the JOS kernel monitor' "$checks/serial.log"; do
	[ "$tick" -lt 150 ] || fail 'JOS did not boot; see serial.log'
	kill -0 "$qemu_pid" 2>/dev/null || fail 'QEMU exited before JOS booted'
	sleep .1
	tick=$((tick + 1))
done
printf '  CHECK   ELF32/libgcc/STABS, JOS boot, GDB breakpoint/registers/step\n'
