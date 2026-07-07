# Ad Libitum uports Notes

`audio/ad-libitum` packages the `ul/ad-libitum` Scheme live coding environment
for an initial Chez Scheme experiment.

## Dependency Status

Manual build/install order:

```text
devel/cmake
lang/chezscheme
audio/libsoundio
audio/ad-libitum
```

Current uports does not automatically execute `BUILD_DEPENDS`, `RUN_DEPENDS`,
or `LIB_DEPENDS`, so run the targets in that order. `devel/cmake` is needed by
`audio/libsoundio`; if it is already installed in another uports root, put that
root's `bin` directory on `PATH`.

The initial port intentionally keeps upstream Chez libraries as submodules
inside this package:

```text
chez-soundio
chez-sockets
chez-portmidi
srfi
violet
```

This is suitable for first bring-up. If these Chez libraries become useful to
other ports, they can be split into separate uports packages later.

## Optional MIDI

PortMidi support is not packaged in the first pass. The upstream project notes
that PortMidi is needed only when using a MIDI controller. Add an
`audio/portmidi` package later if MIDI support is required.

## Build Notes

The port builds the helper dynamic libraries through per-FFI Makefiles copied
into the upstream FFI subdirectories during `post-patch`:

```text
port overlay Makefile for chez-soundio -> $(WRKSRC)/chez-soundio/Makefile
port overlay Makefile for chez-sockets -> $(WRKSRC)/chez-sockets/Makefile
```

Those Makefiles build:

```text
chez-soundio/Makefile:
  ../libbridge.so

chez-sockets/Makefile:
  ../sockets-stub.so
  ../socket-ffi-values.so
```

This keeps FFI-specific build rules next to each FFI binding while keeping the
installed package binary-runtime only. The copied Makefiles are removed from the
staged install.

`SCHEMEH` defaults to the installed Chez Scheme `scheme.h` directory found
under `$(DESTDIR)$(PREFIX)/lib/csv*/`. If auto-detection fails, pass:

```sh
SCHEMEH=/path/to/lib/csv10.2.0/<machine>
```

## Runtime

The package installs sources and helper libraries under:

```text
$(PREFIX)/share/ad-libitum
```

and a launcher:

```text
$(PREFIX)/bin/ad-libitum
```

The launcher changes into the installed Ad Libitum directory and runs Chez
Scheme on `ad-libitum.ss` by default.

## Remote REPL

The port installs a remote REPL launcher mode:

```sh
ad-libitum --remote-repl
```

This mode loads `ad-libitum.ss`, starts the packaged remote REPL server, and
then runs a small non-interactive sleep loop so Chez Scheme continues to
schedule remote REPL accept/eval threads. Starting a remote listener from the
normal local REPL mode is not enough: the local terminal REPL can block Scheme
thread scheduling.

Disable this at package build time with:

```sh
make ADLIB_REMOTE_REPL=no ...
```

The packaged server binds to `127.0.0.1:37146`, so the launcher exposes it only
to local clients. This is intentional because the remote REPL evaluates
arbitrary Scheme expressions in the live Ad Libitum process.

After launching `ad-libitum --remote-repl`, connect with the packaged helper:

```sh
ad-libitum-repl
```

Example session:

```scheme
> (+ 4 5)
9
>
```

The helper reads one local terminal line at a time. For each expression it opens
a fresh local connection to the remote server, sends the expression, prints the
response, and closes the connection. This avoids keeping stale interactive
client sockets attached to the live Ad Libitum process and avoids the buffering
behavior seen with macOS `nc`.

Live audio commands should be evaluated in the same interactive helper session:

```scheme
> (play! tuner)
#<void>
> (h!)
#<void>
>
```

`(play! tuner)` starts the tuner signal and `(h!)` stops it.

For non-interactive smoke testing, use:

```sh
ad-libitum-repl -e '(+ 4 5)'
```

Expected response:

```text
9
```

The rewritten server also responds correctly to raw socket clients. macOS
terminal `nc` can connect and show the prompt, but in testing it buffered typed
input until EOF (`Ctrl-D`), so use `ad-libitum-repl` for normal interactive
work.

Diagnostic `nc` example:

```scheme
(+ 4 5)
```

Then press `Ctrl-D`; expected response:

```text
9
>
```

## Remote REPL Issue Summary

Two remote REPL failures were debugged and fixed.

### Issue 1: REPL Hangs

Symptom:

```scheme
> (+ 4 5)
```

The client could move to the next line but did not receive output or a new
prompt. Earlier tests also showed that raw `nc` could connect and display the
prompt but did not behave reliably as an interactive client.

Root cause:

The remote REPL socket loop was unsafe for Ad Libitum's Chez-threaded runtime.
The failed designs used either Scheme-level socket polling or plain blocking
socket behavior in the wrong place. This could prevent queued expressions from
being processed, so the client waited indefinitely.

Fix:

The final server keeps sockets nonblocking and waits for readiness through a
native `select(2)` wrapper:

```text
nonblocking socket
+ bridge_wait_fd_readable(fd, timeout_usec)
+ accept-socket / receive-from-socket after readiness
```

`bridge_wait_fd_readable` deactivates the current Chez thread while waiting, so
other Scheme threads can continue to run.

### Issue 2: Audio Starts Then Stops

Symptom:

```scheme
> (play! tuner)
#<void>
>
```

A short sound was heard, then output became silent.

The DSP diagnostic was clean:

```scheme
(sound:last-dsp-error)
=> #f
```

Failed remote-mode native counters showed ring-buffer starvation:

```scheme
(frames-copied-from-ring . 16896)
(zero-fill-frames . 1152000)
```

Root cause:

Audio output depends on a Scheme producer thread continuously filling the
libsoundio ring buffer. The native libsoundio callback kept running, but the
remote REPL socket loop starved the Scheme producer thread. The callback then
had to zero-fill most requested frames, producing silence after a short initial
sound.

Fix:

The same nonblocking socket plus native readiness wait fix solved the audio
starvation. After the fix, a real macOS audio session showed:

```scheme
(frames-copied-from-ring . 1897472)
(zero-fill-frames . 0)
(sound:last-dsp-error) => #f
```

### Debug Procedure

Basic remote REPL test:

```scheme
(+ 4 5)
```

Expected:

```scheme
9
```

Audio and native-counter test:

```scheme
(sound:reset-native-status!)
(play! (lambda (time channel) (sin (* 6.283185307179586 time 440.0))))
(sound:status)
(sound:native-status)
(sound:last-dsp-error)
```

Interpretation:

- `(sound:last-dsp-error) => #f`: DSP callback did not throw.
- High `zero-fill-frames`: audio producer starvation or ring-buffer underflow.
- `frames-copied-from-ring` close to requested total: healthy audio flow.
- Increasing `producer-frames-written`: Scheme producer thread is running.

Package/install checks:

```sh
nm -gU local/compiler/usr/local/share/ad-libitum/libbridge.so | \
  rg 'bridge_wait_fd_readable|bridge_soundio_wait_events'
```

Expected:

```text
_bridge_soundio_wait_events
_bridge_wait_fd_readable
```

Clean binary-runtime package check:

```sh
find local/compiler/usr/local/share/ad-libitum \
  -type f \( -name '*.c' -o -name Makefile -o -name 'tangle.sh' \)
```

Expected: no output.

### Modified Code

Main modified runtime/source-tree locations:

```text
ad-libitum/repl.ss
chez-soundio/bridge.c
ad-libitum/sound.ss
chez-soundio/soundio.ss
bin/ad-libitum-repl
chez-soundio/Makefile
chez-sockets/Makefile
```

Change summary:

- `ad-libitum/repl.ss`: remote REPL rewrite, nonblocking sockets,
  `bridge_wait_fd_readable`, queued expression evaluation.
- `chez-soundio/bridge.c`: native wait helper, native audio counters, underflow
  handling, `bridge_soundio_wait_events`.
- `ad-libitum/sound.ss`: DSP error/status/native-status diagnostics.
- `chez-soundio/soundio.ss`: event thread, producer counters, native status access,
  `flush-events`.
- `bin/ad-libitum-repl`: packaged helper client.
- `chez-soundio/Makefile`: builds `../libbridge.so`.
- `chez-sockets/Makefile`: builds `../sockets-stub.so` and
  `../socket-ffi-values.so`.
- `audio/ad-libitum/Makefile`: copies replacement files, delegates FFI builds to per-FFI
  Makefiles, installs runtime files only, removes build-only files from staged
  package payload.
- `audio/ad-libitum/pkg-plist`: matches the cleaned binary-runtime package.

The uports port keeps these replacements under its package overlay, but the
paths above are the source-tree/runtime locations they replace or install.

## Remote REPL Socket Path

The remote REPL has four cooperating layers:

```text
terminal user
  -> bin/ad-libitum-repl
  -> TCP 127.0.0.1:37146
  -> ad-libitum/repl.ss
  -> Ad Libitum live Scheme environment
  -> ad-libitum/sound.ss + chez-soundio/soundio.ss + chez-soundio/bridge.c
```

### Server Startup

The installed launcher supports:

```sh
ad-libitum --remote-repl
```

That mode changes into the installed Ad Libitum runtime directory and runs the
generated script:

```text
ad-libitum-remote-repl.ss
```

The generated script does:

```scheme
(load "ad-libitum.ss")
(repl:start-repl-server)
(let loop ()
  (sound:flush-events!)
  (repl:process-pending!)
  (sleep ...)
  (loop))
```

This is intentionally not the normal local Chez REPL. The loop keeps the live
process alive, flushes soundio events, and processes queued remote expressions.

### Client Path

The packaged client is:

```text
bin/ad-libitum-repl
```

It connects to:

```text
127.0.0.1:37146
```

The client protocol is line-oriented:

1. connect to the local TCP server;
2. read the initial `> ` prompt;
3. read one local terminal line;
4. send that line plus newline;
5. read until the next `> ` prompt;
6. print the response.

Example:

```scheme
> (+ 4 5)
9
>
```

Raw `nc` can connect, but macOS `nc` buffered input during testing and was not
reliable as the normal interactive client.

### Server Socket Path

The server implementation lives at:

```text
ad-libitum/repl.ss
```

It creates an internet stream socket, binds only to localhost, and listens:

```text
127.0.0.1:37146
```

The server socket and accepted client sockets are kept nonblocking. Before
calling `accept-socket` or `receive-from-socket`, the server calls:

```scheme
(wait-readable socket)
```

`wait-readable` calls the native FFI helper:

```scheme
bridge_wait_fd_readable
```

The important design is:

```text
nonblocking socket
+ native select(2) wait
+ Chez thread deactivation while waiting
```

This avoids both bad earlier behaviors:

- busy polling in Scheme;
- plain blocking socket calls that can stop the remote REPL from responding.

### FFI Wait Path

The native helper is implemented at:

```text
chez-soundio/bridge.c
```

It wraps `select(2)`:

```c
Sdeactivate_thread();
ret = select(fd + 1, &read_fds, NULL, NULL, timeout_ptr);
Sactivate_thread();
```

`Sdeactivate_thread()` tells Chez that the current Scheme thread is waiting in
C. This allows other Scheme threads to keep running while the socket waits for
readability.

That detail is critical for audio. The sound producer is a Scheme thread; if
socket waiting starves Scheme scheduling, the libsoundio ring buffer drains and
the native callback has to output zeros.

### Expression Evaluation Path

When bytes arrive, `ad-libitum/repl.ss` decodes them and appends them to a line
buffer. A complete line is read as one Scheme expression:

```scheme
(call-with-port (open-string-input-port line) read)
```

The socket path does not directly evaluate the expression. It queues a request:

```text
expr + output slot + done? flag
```

The main remote loop later calls:

```scheme
(repl:process-pending!)
```

That drains the queue, evaluates each expression with `eval`, stores the output,
and marks the request done. The client handler then sends the result and the
next prompt.

For:

```scheme
(+ 4 5)
```

the flow is:

```text
bin/ad-libitum-repl sends "(+ 4 5)\n"
  -> ad-libitum/repl.ss waits for socket readability
  -> receive-from-socket reads bytes
  -> line parser reads one Scheme expression
  -> request is queued
  -> repl:process-pending! evaluates expression
  -> result "9\n" is sent back
  -> client prints result and prompt
```

### Audio Interaction

Audio output uses:

```text
ad-libitum/sound.ss
chez-soundio/soundio.ss
chez-soundio/bridge.c
```

The live audio path is:

```text
Scheme DSP procedure
  -> Scheme producer thread fills libsoundio ring buffer
  -> native libsoundio write callback consumes ring buffer
  -> output device
```

For:

```scheme
(play! tuner)
```

the flow is:

```text
remote REPL receives expression
  -> request is queued
  -> repl:process-pending! evaluates play!
  -> ad-libitum/sound.ss updates current DSP procedure
  -> chez-soundio/soundio.ss producer thread writes samples
  -> chez-soundio/bridge.c native callback copies samples to libsoundio
```

The final socket design lets the socket thread wait without starving the Scheme
producer thread. That is why the final remote-mode native counters changed from
mostly zero-fill frames to:

```scheme
(frames-copied-from-ring . 1897472)
(zero-fill-frames . 0)
```

### Build-Time FFI Layout

The FFI build rules are also organized by source-tree location:

```text
chez-soundio/Makefile -> ../libbridge.so
chez-sockets/Makefile -> ../sockets-stub.so
chez-sockets/Makefile -> ../socket-ffi-values.so
```

These Makefiles are used at package build time only. The installed package is a
binary-runtime package: it installs the `.so` libraries and Scheme runtime
sources, but not the C source files or Makefiles.

## Remote REPL Root Cause Analysis

### Confirmed Result

The remote REPL is now validated for both arithmetic evaluation and live audio.
The final supported implementation uses this design:

```text
nonblocking TCP sockets
+ native select(2) readability wait
+ Chez thread deactivation around the native wait
+ queued evaluation in the main Ad Libitum loop
```

Validation from a real macOS audio session:

```scheme
> (+ 4 5)
9
> (play! (lambda (time channel)
           (sin (* 6.283185307179586 time 440.0))))
#<void>
```

Audio kept playing. Runtime counters showed that the native audio callback no
longer had to output silence:

```scheme
(sound:native-status)
=> ((write-callback-calls . 3706)
    (frames-requested-min-total . 1897472)
    (frames-requested-max-total . 1897472)
    (frames-copied-from-ring . 1897472)
    (zero-fill-frames . 0)
    ...)

(sound:last-dsp-error)
=> #f
```

The important values are:

```text
frames-copied-from-ring == frames requested by libsoundio
zero-fill-frames        == 0
sound:last-dsp-error    == #f
```

This proves that the remote REPL is no longer starving the Scheme audio
producer and that the failure was not caused by the DSP procedure itself.

### Root Cause

The root cause was the remote REPL socket loop interfering with Ad Libitum's
thread-driven audio producer. Ad Libitum's audio path has two active parts:

1. libsoundio invokes a native write callback and consumes frames from a ring
   buffer;
2. a Scheme producer thread continuously computes DSP samples and fills that
   ring buffer.

When the remote REPL used Scheme-level socket polling or unsafe blocking socket
behavior, the Scheme producer did not make enough progress. The native callback
continued to run, but it often found the ring buffer effectively empty and had
to zero-fill most frames. That produced the symptom:

```text
(play! tuner) returns #<void>, a short sound is heard, then output becomes silent
```

Representative failed remote-mode counters before the final fix:

```scheme
(sound:native-status)
=> ((frames-copied-from-ring . 16896)
    (zero-fill-frames . 1152000)
    ...)
```

The DSP error recorder returned `#f`, so the signal function was not raising an
exception. The problem was starvation below the DSP expression layer.

### Failed Alternatives

The investigation deliberately avoided treating this as a client-helper issue
after arithmetic started working. These alternatives were tested and rejected:

- Python client changes alone: arithmetic could work, but audio still stopped.
- Raw `nc` testing: useful for observing prompts, but macOS `nc` buffering made
  it a poor interactive client for this protocol.
- Plain blocking sockets: avoided busy polling but caused the REPL to hang on
  `(+ 4 5)` in the full remote server path.
- Increasing audio latency/ring-buffer behavior: could lengthen the audible
  burst, but did not fix the producer starvation.
- Adding `__collect_safe` only to the bundled `chez-sockets` blocking FFI
  declarations: did not fix the receive/thread failure in the tested runtime.

### Accepted Fix

The accepted fix keeps the sockets in nonblocking mode and adds an explicit
native wait primitive in `libbridge.so`:

```c
EXPORT long bridge_wait_fd_readable(int fd, long timeout_usec) {
  ...
  Sdeactivate_thread();
  ret = select(fd + 1, &read_fds, NULL, NULL, timeout_ptr);
  Sactivate_thread();
  ...
}
```

The Scheme REPL server uses that wait before `accept-socket` and
`receive-from-socket`:

```scheme
(define bridge_wait_fd_readable
  (foreign-procedure "bridge_wait_fd_readable" (int long) long))

(define (wait-readable socket)
  (let loop ()
    (let ([result (bridge_wait_fd_readable
                   (sock:socket-fd socket)
                   socket-wait-timeout-usec)])
      (cond
       [(positive? result) #t]
       [(zero? result) (loop)]
       [else (error 'wait-readable "select failed" result)]))))
```

This preserves the `chez-sockets` nonblocking API behavior while avoiding a hot
Scheme polling loop. The native wait deactivates the current Chez thread while
waiting, so other Scheme threads, including the audio producer, continue to run.

### Supported Workflow

Start the remote server:

```sh
DYLD_LIBRARY_PATH=$PWD/local/compiler/usr/local/lib \
  $PWD/local/compiler/usr/local/bin/ad-libitum --remote-repl
```

Connect with the packaged helper:

```sh
$PWD/local/compiler/usr/local/bin/ad-libitum-repl
```

Smoke test arithmetic:

```scheme
> (+ 4 5)
9
>
```

Start and stop audio:

```scheme
> (play! tuner)
#<void>
> (h!)
#<void>
>
```

For diagnosing audio problems, reset and read the native counters:

```scheme
(sound:reset-native-status!)
(play! (lambda (time channel) (sin (* 6.283185307179586 time 440.0))))
(sound:status)
(sound:native-status)
(sound:last-dsp-error)
```

A healthy remote run has `zero-fill-frames` at or near zero after startup and
`(sound:last-dsp-error)` returning `#f`.

### Packaging Notes

The replacement Scheme runtime files are copied in both `post-patch` and
`do-install`. The install-time copy is intentional because this port may use an
existing extracted or symlinked source tree under `local/compiler/src`;
staged/package contents must still match the port's runtime replacements during
incremental work.

The package is a binary-runtime package for the FFI helpers. It installs the
compiled libraries:

```text
libbridge.so
sockets-stub.so
socket-ffi-values.so
```

It does not install build-only C sources or helper make scripts such as:

```text
chez-soundio/bridge.c
chez-soundio/sine.c
chez-soundio/tangle.sh
chez-sockets/Makefile
chez-sockets/socket-ffi-values.c
chez-sockets/sockets-stub.c
```

When changing the `chez-soundio/bridge.c` replacement, force the real source
tree through the patch and build phases before packaging. Then verify the
installed runtime Scheme files and the compiled bridge symbols:

```sh
cmp -s <port-overlay>/ad-libitum/repl.ss \
  local/compiler/usr/local/share/ad-libitum/ad-libitum/repl.ss

nm -gU local/compiler/usr/local/share/ad-libitum/libbridge.so | \
  rg 'bridge_wait_fd_readable|bridge_soundio_wait_events'
```

Expected symbol check:

```text
_bridge_soundio_wait_events
_bridge_wait_fd_readable
```

Final packaging validation:

```text
pkg-plist matches staged install as a set
package payload has no *.c files, Makefile, or chez-soundio/tangle.sh
installed tree has no *.c files, Makefile, or chez-soundio/tangle.sh
```

### DSP Callback Diagnostics

The original `(ad-libitum sound)` callback swallowed all DSP exceptions and
returned silence:

```scheme
(guard (_ [else 0.0])
  (*dsp* time channel))
```

That made `(play! tuner)` failures indistinguishable from a valid silent signal
or a stopped stream. The packaged port now replaces `ad-libitum/sound.ss` with a
diagnostic version that:

- clears the previous DSP error when `set-dsp!` installs a new DSP;
- records the first callback exception in `sound:last-dsp-error`;
- prints the first callback exception to the Ad Libitum server's stderr;
- still returns `0.0` after an exception so the audio thread can continue far
  enough for diagnosis.

After starting `ad-libitum --remote-repl`, test from `ad-libitum-repl`:

```scheme
(sound:last-dsp-error)
(play! tuner)
(sound:last-dsp-error)
```

If the second `sound:last-dsp-error` returns a string, that string is the real
DSP callback failure. If it returns `#f` but audio still stops, the problem is
below the DSP procedure path, likely in the soundio write-thread or stream state.

Automated validation performed in this environment:

```text
stage ad-libitum/sound.ss     == port replacement  PASS
package ad-libitum/sound.ss   == port replacement  PASS
installed ad-libitum/sound.ss == port replacement  PASS
parser-level read of ad-libitum/sound.ss           PASS
```

Audible validation requires a terminal/session with a working output device.
In this tool environment, libsoundio aborts during default output device
initialization before the REPL server starts, but a user terminal validated
that remote audio now keeps playing:

```text
Assertion failed: (index < si->safe_devices_info->output_devices.length),
function soundio_get_output_device, file soundio.c, line 381.
```
