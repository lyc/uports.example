# Ports collection      ChezScheme
# Date created:         2023/06/10
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= ChezScheme
DISTVERSIONPREFIX	= v
DISTVERSION 		= 10.4.1
CATEGORIES 		= lang
USE_GITHUB		= yes
GH_ACCOUNT		= cisco
WRKSRC			= $(WRKDIR)/$(PORTNAME)-$(DISTVERSION)
DIST_SUBDIR		= $(PORTNAME)

# Archive mode assembles the subprojects at the exact v10.4.1 gitlink commits.
# Update these pins together with DISTVERSION. Git mode uses SCM_SUBMODULES.
GH_TUPLE		= $(if $(filter dist,$(SOURCE_MODE_EFFECTIVE)), \
	lz4:lz4:ebb370ca83af193212df4dcbadcc5d87bc0de2f0:lz4/lz4	\
	nanopass:nanopass-framework-scheme:bb47b5695a5bffdfd4d99ec9b349f0864b08e0d4:nanopass/nanopass \
	dybvig:stex:afa607564a5662ffd748e824801277a6b5a3d11c:stex/stex	\
	madler:zlib:da607da739fa6047df13e66a2af6b8bec7c2a498:zlib/zlib	\
	racket:zuo:a288cbfe350403899311ca7d8267d570e781ed2c:zuo/zuo)

USE_SCM			?= git
SCM_CMD_OPTS		= --filter=blob:none
SCM_REPO_URL		= https://github.com/cisco/$(PORTNAME).git
SCM_DEST		= $(PORTNAME)-$(DISTVERSION)
SCM_DETACH		= $(DISTVERSIONFULL)
SCM_SUBMODULES		= $(if $(filter git,$(SOURCE_MODE_EFFECTIVE)),	\
			    lz4 nanopass stex zlib zuo)

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

override_targets 	+= post-install

post-install:
ifeq ($(OPSYS),darwin)
	@for f in `find $(STAGEDIR)$(PREFIX)/lib/csv$(DISTVERSION) -type f \
	    \( -name scheme -o -name petite -o -name scheme-script \)`; do \
	    install_name_tool -rpath $(DESTDIR)$(PREFIX)/lib @loader_path/../.. "$$f" \
	        2>/dev/null || true;					\
	done
endif

include $(PORTSDIR)/Mk/linux.port.post.mk
