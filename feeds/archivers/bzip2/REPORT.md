# bzip2 Port Report

Bootstrap classification: none. The upstream Makefile builds the compressor,
static library and compression roundtrip checks using the selected C compiler.

Linux additionally builds the shared library using Makefile-libbz2_so. That
makefile uses ELF linker flags and must not be invoked on Darwin.

Darwin links the PIC objects from the ordinary build with -dynamiclib,
producing libbz2.1.0.8.dylib with install name @rpath/libbz2.1.0.dylib,
compatibility version 1.0 and current version 1.0.8. Installation adds
libbz2.1.0.dylib and libbz2.dylib symlinks. The static library and compressor
installation remain shared between platforms. No DESTDIR or build path is
embedded in the dylib install name; PREFIX selects the installation layout.
Linking reserves load-command space with -headerpad_max_install_names for
the framework's Darwin fixup, which sets the ID to the full dylib basename
and adds the final-prefix library rpath. The final staged ID is therefore
@rpath/libbz2.1.0.8.dylib. Rebuild the dylib after changing these link flags;
restaging the old binary cannot add the missing header space.

Darwin build/stage and runtime validation are pending. The existing Darwin
plist still lists Linux shared-library names: regenerate it from the completed
Darwin stage before packaging/installing. Inspect the staged dylib with
otool -D and otool -L; validate a small C program using BZ2_bzBuffToBuffCompress
and BZ2_bzBuffToBuffDecompress linked against the staged library, with its
staged lib directory supplied through DYLD_LIBRARY_PATH. Check a real
compress/decompress roundtrip rather than only the executable version.
