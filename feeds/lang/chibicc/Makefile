# Ports collection      chibicc
# Date created:         2025/10/23
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= chibicc
DISTVERSION		= 20201207
CATEGORIES 		= lang
CHIBICC_COMMIT		= 90d1f7f199cc55b13c7fdb5839d1409806633fdb
USE_GITHUB		= yes
GH_ACCOUNT		= rui314
GH_TAGNAME		= $(CHIBICC_COMMIT)
DIST_SUBDIR		= $(PORTNAME)

# Set SOURCE_MODE=dist to use the upstream commit snapshot tarball.
USE_SCM			?= git
SCM_REPO_URL		= https://github.com/rui314/$(PORTNAME).git
SCM_DETACH		= $(CHIBICC_COMMIT)
SCM_RETRIEVE_TIMEOUT	= 300

MAINTAINER 		= yowching.lee@gmail.com
COMMENT			= Small educational C11 compiler for x86-64 Linux

LICENSE			= MIT
LICENSE_FILE		= $(WRKSRC)/LICENSE

ALL_TARGET		= chibicc
MAKE_ARGS		+= CC="$(CC)"					\
			   CFLAGS="-std=c11 -O2 -fno-common -Wall	\
				   -Wno-switch $(CFLAGS)"		\
			   LDFLAGS="$(LDFLAGS)"

#
#
#

MASTERDIR 		= $(CURDIR)
PORTSDIR 		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= do-install

do-install:
	@mkdir -p $(STAGEDIR)$(PREFIX)/bin
	@mkdir -p $(STAGEDIR)$(PREFIX)/libexec/$(PORTNAME)/include
	@mkdir -p $(STAGEDIR)$(PREFIX)/share/doc/$(PORTNAME)
	@cp $(WRKSRC)/$(PORTNAME)					\
	    $(STAGEDIR)$(PREFIX)/libexec/$(PORTNAME)/$(PORTNAME)
	@chmod 755 $(STAGEDIR)$(PREFIX)/libexec/$(PORTNAME)/$(PORTNAME)
	@cp $(WRKSRC)/include/*.h					\
	    $(STAGEDIR)$(PREFIX)/libexec/$(PORTNAME)/include
	@cp $(WRKSRC)/LICENSE $(WRKSRC)/README.md			\
	    $(STAGEDIR)$(PREFIX)/share/doc/$(PORTNAME)
	@printf '%s\n'							\
	    '#!/bin/sh'							\
	    'bindir=$$(dirname "$$0")'					\
	    'prefix=$$(cd "$$bindir/.." && pwd)'			\
	    'exec "$$prefix/libexec/$(PORTNAME)/$(PORTNAME)" "$$@"' 	\
	    > $(STAGEDIR)$(PREFIX)/bin/$(PORTNAME)
	@chmod 755 $(STAGEDIR)$(PREFIX)/bin/$(PORTNAME)

include $(PORTSDIR)/Mk/linux.port.post.mk
