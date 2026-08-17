# Ports collection:      boehm-gc

PORTNAME		= boehm-gc
DISTVERSION		= 8.2.12
CATEGORIES		= devel
MASTER_SITES		= https://github.com/bdwgc/bdwgc/releases/download/v$(DISTVERSION)/
DISTNAME		= gc-$(DISTVERSION)
DISTFILES		= $(DISTNAME).tar.gz
DIST_SUBDIR		= $(PORTNAME)

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= Garbage collector for C and C++

LICENSE			= BDWGC
LICENSE_FILE		= $(WRKSRC)/LICENSE

USES			= libtool

GNU_CONFIGURE		= yes
# Upstream prepends hidden visibility after its feature probe.  Restoring the
# default at the end fixes libcord's partial-link step on arm64 Darwin.
ifeq ($(shell uname -s),Darwin)
CFLAGS			+= -fvisibility=default
endif
CONFIGURE_ARGS		+= --enable-threads=posix			\
			   --enable-thread-local-alloc
INSTALL_TARGET		= install-strip

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.mk
