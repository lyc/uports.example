# Ports collection      ChezScheme
# Date created:         2023/06/10
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= ChezScheme
DISTVERSIONPREFIX	= v
DISTVERSION 		= 10.2.0
CATEGORIES 		= lang
MASTER_SITES 		= $(MASTER_SITE_GITHUB)
MASTER_SITE_SUBDIR	= cisco/$(PORTNAME)
DISTNAME		= $(DISTVERSIONFULL)
WRKSRC			= $(WRKDIR)/$(PORTNAME)-$(DISTVERSION)
DIST_SUBDIR		= $(PORTNAME)

USE_SCM			= git
SCM_CMD_OPTS		= --filter=blob:none
SCM_PROTOCOL		= https://
MASTER_SITES		= github.com/
MASTER_SITE_SUBDIR	= cisco
SCM_DEST		= $(PORTNAME)-$(DISTVERSION)
SCM_DETACH		= $(DISTVERSIONFULL)

MAINTAINER 		= yowching.lee@gmail.com
COMMENT			= Chez Scheme is both a programming language and an implementation of that language.

LICENSE			= Apache-2.0
LICENSE_FILE		= $(WRKSRC)/LICENSE

LIB_DEPENDS		= libncurses.so:devel/ncurses			\
			  libffi.so:devel/libffi
ifneq ($(shell uname -s),Darwin)
LIB_DEPENDS		+= libiconv.so:converters/libiconv
endif

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
CONFIGURE_ARGS		+= --disable-iconv
endif
ALL_TARGET		=

#
#
#

MASTERDIR 		= $(CURDIR)
PORTSDIR 		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets 	+= post-patch post-install

post-patch:
	@cd $(WRKSRC);							\
	git submodule init;						\
	git submodule update

post-install:
ifeq ($(OPSYS),darwin)
	@for f in `find $(STAGEDIR)$(PREFIX)/lib/csv$(DISTVERSION) -type f \
	    \( -name scheme -o -name petite -o -name scheme-script \)`; do \
	    install_name_tool -rpath $(DESTDIR)$(PREFIX)/lib @loader_path/../.. "$$f" \
	        2>/dev/null || true;					\
	done
endif

include $(PORTSDIR)/Mk/linux.port.post.mk
