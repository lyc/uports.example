# Ports collection      cpython
# Date created:         2025/08/13
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= cpython
DISTVERSIONPREFIX	= v
DISTVERSION 		= 3.14.5
CATEGORIES 		= lang
MASTER_SITES 		= https://www.python.org/ftp/python/$(DISTVERSION)/
DISTNAME		= Python-$(DISTVERSION)
WRKSRC			= $(WRKDIR)/$(PORTNAME)-$(DISTVERSION)

USE_SCM			?= git
ifeq ($(USE_SCM),git)
SCM_PROTOCOL		= https://
MASTER_SITES		= github.com/
MASTER_SITE_SUBDIR	= python
SCM_DEST		= $(PORTNAME)-$(DISTVERSION)
SCM_DETACH		= $(DISTVERSIONFULL)
endif

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
			  libgdbm.so:databases/gdbm
ifeq ($(shell uname -s),Linux)
LIB_DEPENDS		+= libuuid.so:devel/libuuid
endif

USES			= tar:xz

EXTRACT_TRANSFORM	= /Python/cpython/g

GNU_CONFIGURE 		= yes
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
