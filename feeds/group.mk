#
# Project groups, toolchain profiles and dependency-provider policy.
#

default_ignore_lists	+=

ignore_lists		+=

# Groups and environments.

PORTS_GROUPS		= compiler bs host textproc toolchain

# Project export tooling extends the framework's host group.
host_lists		+= sysutils/patchelf

# Group membership selects the development environment.
compiler_lists		= devel/libffi archivers/zlib math/gmp	       \
	math/mpfr math/mpc devel/ncurses devel/readline		       \
	security/openssl databases/sqlite databases/gdbm	       \
	devel/libuuid devel/libffcall devel/boehm-gc devel/libltdl     \
	archivers/zstd archivers/xz archivers/bzip2		       \
	audio/ad-libitum audio/libsoundio lang/quickjs lang/pforth     \
	lang/chezscheme lang/gforth-0.7.3 lang/guile lang/gambit       \
	lang/sbcl-bootstrap lang/lua lang/chibi-scheme lang/gforth     \
	lang/clisp lang/cpython lang/elk lang/sbcl

textproc_lists		= textproc/ezxml			       \
			  textproc/expat2			       \
			  textproc/mxml				       \
			  textproc/json-c			       \
			  textproc/libcsv			       \
			  converters/libiconv			       \
			  devel/libunistring			       \
			  devel/pcre2				       \
			  net/libyang2

PORTS_textproc_ENVS	+= $(PORTS_host_ENVS)			       \
			   $(strip				       \
			     PORTSDIR=$(portdir)		       \
			     PREFIX=$(PREFIX)			       \
			     DESTDIR=$(DESTDIR)/$(PORTS_GROUP_DEFAULT) \
			     $(if $(USE_ALTERNATIVE),		       \
			       USE_ALTERNATIVE=$(USE_ALTERNATIVE),     \
			       USE_ALTERNATIVE=yes)		       \
			     $(if $(ALTERNATIVE_WRKDIR),	       \
			       ALTERNATIVE_WRKDIR=$(ALTERNATIVE_WRKDIR),\
			       ALTERNATIVE_WRKDIR=$(DESTDIR)/$(PORTS_GROUP_DEFAULT)/src))

# Native tool producers coexist with the normal compiler environment.
# Profile recipes own their target tuples and PREFIX defaults.
# Keep the legacy PREFIX default for group introspection; producers omit it.
# CROSS_COMPILE stays unset because these executables run on the host.
$(eval $(call uports-native-group,toolchain))

# Toolchain membership and default Flavors.

TOOLCHAIN_PORTS		?= devel/binutils			       \
			   lang/gcc				       \
			   devel/gdb				       \
			   emulators/qemu-mit			       \
			   emulators/qemu

# Missing planned recipes stay inactive, including before the first recipe
# commit. Add a recipe only after it supports the selections configured here.
toolchain_lists		= $(foreach p,$(TOOLCHAIN_PORTS),	       \
	$(if $(wildcard $(feeds)/$p/Makefile $(portdir)/$p/Makefile),$p))

# Native emulator prerequisites use the ordinary /usr/local host-tool layout.
toolchain_lists		+= $(foreach p,				       \
	lang/python27 devel/glib20 x11/pixman devel/libffi	       \
	archivers/zlib devel/gettext,				       \
	$(if $(wildcard $(feeds)/$p/Makefile $(portdir)/$p/Makefile),$p))

# Expose native math providers independently of the selected GCC Flavor.
# The project setup can precede recipe commits without missing members.
toolchain_math_lists	= $(if $(wildcard			       \
	$(feeds)/lang/gcc/Makefile $(portdir)/lang/gcc/Makefile),      \
	math/gmp math/mpfr math/mpc)
toolchain_lists		+= $(toolchain_math_lists)

# Aggregate selections are scoped to the toolchain group only.
PORTS_toolchain_binutils_EXTRA_ENVS += FLAVOR=mit2018
PORTS_toolchain_gcc_EXTRA_ENVS += FLAVOR=mit2018
PORTS_toolchain_gdb_EXTRA_ENVS += FLAVOR=mit2018
PORTS_toolchain_qemu-mit_EXTRA_ENVS += FLAVOR=mit2018
PORTS_toolchain_qemu_EXTRA_ENVS += FLAVOR=mit2026
$(foreach p,$(notdir $(toolchain_math_lists)),			       \
  $(eval PORTS_toolchain_$p_EXTRA_ENVS += FLAVOR=mit2018))

$(foreach p,binutils gcc gdb qemu-mit qemu gmp mpfr mpc,	       \
  $(eval PORTS_toolchain_$p_EXCLUDE_ENVS += PREFIX=%))

# ARM64 target libraries (devel/linux-headers and devel/glibc) need a separate
# target/sysroot context. Its policy and GCC bootstrap-stage selections remain
# deferred to the reviewed stage design; do not put them in this native group.

# Dependency-provider policy. Explicit caller policy files load later and
# may override these defaults through UPORTS_DEPENDENCY_POLICY.

# Native tools/libraries use validated system providers with uports fallback.
DEPENDENCY_PROVIDER_MODE = host

# Host dependencies remain restricted to native system providers or host/bs.
DEPENDENCY_PROVIDER_UPORTS_GROUPS.host = host bs
DEPENDENCY_PROVIDER_UPORTS_GROUPS.toolchain = toolchain host bs compiler
