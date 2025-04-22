# Ports collection      ChezScheme
# Date created:         2023/06/10
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= ChezScheme
DISTVERSIONPREFIX	= v
DISTVERSION 		= 10.0.0
CATEGORIES 		= lang
MASTER_SITES 		= $(MASTER_SITE_GITHUB)
MASTER_SITE_SUBDIR	= cisco/$(PORTNAME)
DISTNAME		= $(DISTVERSIONFULL)
WRKSRC			= $(WRKDIR)/$(PORTNAME)-$(DISTVERSION)
DIST_SUBDIR		= $(PORTNAME)

MAINTAINER 		= yowching.lee@gmail.com
COMMENT			= Chez Scheme is both a programming language and an implementation of that language.

LICENSE			= Apache-2.0
LICENSE_FILE		= $(WRKSRC)/LICENSE

LIB_DEPENDS		= libncurses.so:devel/ncurses			\
			  libffi.so:devel/libffi
ifneq ($(shell uname -s),Darwin)
LIB_DEPENDS		+= libiconv.so:converters/libiconv
endif

USE_SCM			= git
SCM_CMD_OPTS		= --filter=blob:none
SCM_PROTOCOL		= https://
MASTER_SITES		= github.com/
MASTER_SITE_SUBDIR	= cisco
SCM_DEST		= $(PORTNAME)-$(DISTVERSION)
SCM_DETACH		= $(DISTVERSIONFULL)

# NOTE:
#   Use "GNU_CONFIGURE" to get $(CFLAGS) and $(LDFLAGS) configuration,
#   although it is not GNU configure
GNU_CONFIGURE		= yes
CONFIGURE_ARGS		+= --enable-libffi				\
			   --disable-x11				\
			  --installprefix=$(DESTDIR)$(PREFIX)		\
			  --temproot=$(WRKDIR)/stage
ifneq ($(shell uname -s),Darwin)
MAKE_ENV		+= LD_LIBRARY_PATH=$(DESTDIR)$(PREFIX)/lib
else
# NOTE: need to disable macOS SIP (System Integrity Protection)
MAKE_ENV		+= DYLD_FALLBACK_LIBRARY_PATH=$(DESTDIR)$(PREFIX)/lib
CONFIGURE_ARGS		+= --disable-iconv
endif
ALL_TARGET		=

#
#
#

MASTERDIR 		= $(CURDIR)
PORTSDIR 		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets 	+= post-patch

post-patch:
	@cd $(WRKSRC);							\
	git submodule init;						\
	git submodule update

include $(PORTSDIR)/Mk/linux.port.post.mk
