# libsoundio uports Notes

`audio/libsoundio` packages libsoundio 2.0.0, matching the dependency version
noted by the Chez Scheme `chez-soundio` bindings used by Ad Libitum.

The port uses the upstream CMake build and disables examples/tests for the
initial package. On Darwin, libsoundio uses the platform CoreAudio backend; on
Linux, optional audio backends can be revisited after the first Ad Libitum
bring-up.

Manual bootstrap order for Ad Libitum experiments:

```text
devel/cmake
lang/chezscheme
audio/libsoundio
audio/ad-libitum
```

Current uports does not automatically execute dependency targets, so build and
install these in order. If `devel/cmake` is already installed in a different
uports root, put its `bin` directory on `PATH` while building `audio/libsoundio`.
