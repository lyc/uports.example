"""Historical MIT 2018 JOS composition and source preparation settings."""

from common import prepare_source


TUPLE = "i386-jos-elf"
PREFIX = "/i386-jos-elf"
DESCRIPTION = "Historical MIT 2018 JOS toolchain"
VALIDATION = "relocated compiler/libgcc, JOS boot, GDB breakpoint/step"
DATABASE_ROOTS = ("", "compiler")
NATIVE_PREFIXES = ("usr/local", "compiler/usr/local")
# Nested target/bin contains host Binutils; target libraries are not host tools.
HOST_PATHS = ("bin", "libexec", "i386-jos-elf/bin")
TERMINAL_DATA = True
VALIDATORS = ("runtime", "compiler", "jos")
VALIDATION_CONFIG = {"TARGET": TUPLE, "GCC_VERSION": "4.6.4"}
SOURCE_REPOSITORY = "https://pdos.csail.mit.edu/6.828/2018/jos.git"
SOURCE_COMMIT = "a56269d4beefc7d0b3672180aa46c654cfb63af4"
SOURCE_FILES = ("GNUmakefile",)
SOURCE_GIT = True
SOURCE_METADATA = {"jos_commit": SOURCE_COMMIT}
COMPONENTS = {
    "devel/binutils": "binutils-mit2018-2.21.1",
    "lang/gcc": "gcc-mit2018-4.6.4",
    "devel/gdb": "gdb-mit2018-7.3.1",
    "emulators/qemu-mit": "qemu-mit-mit2018-2.3.0.20180907",
}


def default_source(project, destdir, environment):
    source = destdir / "src/jos-validation" / SOURCE_COMMIT
    return prepare_source(project, source, SOURCE_REPOSITORY, SOURCE_COMMIT,
                          environment)
