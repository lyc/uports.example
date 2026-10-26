# Ports collection      cpython
# Date created:         2025/08/13
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= cpython
DISTVERSIONPREFIX	= v
DISTVERSION 		= 3.14.5
CATEGORIES 		= lang
# Keep the official python.org release bundle for dist mode. USE_GITHUB=yes
# would select a generated tag archive with different checksums and potentially
# different generated files; adopting it requires separate source validation.
MASTER_SITES 		= https://www.python.org/ftp/python/$(DISTVERSION)/
DISTNAME		= Python-$(DISTVERSION)
WRKSRC			= $(WRKDIR)/$(PORTNAME)-$(DISTVERSION)

USE_SCM			?= git
# Keep the release archive available independently of Git source selection.
SCM_REPO_URL		= https://github.com/python/cpython.git
SCM_DEST		= $(PORTNAME)-$(DISTVERSION)
SCM_DETACH		= $(DISTVERSIONFULL)

MAINTAINER 		= yowching.lee@gmail.com
COMMENT			= The Python programming language

LICENSE			= PFS
LICENSE_FILE		= $(WRKSRC)/LICENSE

LIB_DEPENDS		= libz.so:archivers/zlib			\
			  libbz2.so:archivers/bzip2			\
			  liblzma.so:archivers/xz			\
			  libzstd.so:archivers/zstd			\
			  libssl.so:security/openssl			\
			  libsqlite3.so:databases/sqlite		\
			  libreadline.so:devel/readline			\
			  libffi.so:devel/libffi			\
			  libncurses.so:devel/ncurses			\
			  libgdbm.so:databases/gdbm
ifeq ($(shell uname -s),Linux)
LIB_DEPENDS		+= libuuid.so:devel/libuuid
endif

USES			= tar:xz

# Rename only the archive root; Include/Python.h must retain its name.
EXTRACT_TRANSFORM	= /^Python-/cpython-/

GNU_CONFIGURE 		= yes
ifeq ($(shell uname -s),Darwin)
# New SDKs expose these before the deployment target supports them.
# CPython 3.14.5 has no Darwin availability guards for these calls.
CONFIGURE_ENV		+= ac_cv_func_dup3=no ac_cv_func_pipe2=no
endif
OPENSSL_LIBDIR		= $(if $(wildcard $(DESTDIR)$(PREFIX)/lib64/libssl.so),lib64,lib)
CONFIGURE_ARGS		+= --with-openssl=$(DESTDIR)$(PREFIX)		\
			   --with-openssl-rpath=$(DESTDIR)$(PREFIX)/$(OPENSSL_LIBDIR)
MAKE_ENV		+= LD_LIBRARY_PATH=$(DESTDIR)$(PREFIX)/lib:$(DESTDIR)$(PREFIX)/lib64

#
#
#

MASTERDIR 		= $(CURDIR)
PORTSDIR 		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.mk
