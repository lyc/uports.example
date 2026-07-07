# Ports collection      pforth
# Date created:         2026/07/06
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= pforth
DISTVERSIONPREFIX	= v
DISTVERSION 		= 2.0.1
CATEGORIES 		= lang
MASTER_SITES 		= https://github.com/philburk/$(PORTNAME)/archive/refs/tags/
DISTFILES		= $(DISTVERSIONFULL)$(EXTRACT_SUFX)
DISTNAME		= $(PORTNAME)-$(DISTVERSION)
WRKSRC			= $(WRKDIR)/$(PORTNAME)-$(DISTVERSION)
DIST_SUBDIR		= $(PORTNAME)

USE_SCM			?= git
ifeq ($(USE_SCM),git)
SCM_PROTOCOL		= https://
MASTER_SITES		= github.com/
MASTER_SITE_SUBDIR	= philburk
SCM_DEST		= $(DISTNAME)
SCM_DETACH		= $(DISTVERSIONFULL)
endif

MAINTAINER 		= yowching.lee@gmail.com
COMMENT			= Portable ANS-like Forth written in ANSI C

LICENSE			= 0BSD
LICENSE_FILE		= $(WRKSRC)/license.txt

BUILD_WRKSRC		= $(WRKSRC)/platforms/unix
INSTALL_WRKSRC		= $(WRKSRC)/platforms/unix

ALL_TARGET		= all

#
#
#

MASTERDIR 		= $(CURDIR)
PORTSDIR 		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= do-install

do-install:
	@mkdir -p $(STAGEDIR)$(PREFIX)/bin
	@mkdir -p $(STAGEDIR)$(PREFIX)/libexec/$(PORTNAME)
	@mkdir -p $(STAGEDIR)$(PREFIX)/share/$(PORTNAME)
	@mkdir -p $(STAGEDIR)$(PREFIX)/share/doc/$(PORTNAME)
	@cp $(BUILD_WRKSRC)/pforth $(STAGEDIR)$(PREFIX)/libexec/$(PORTNAME)/pforth-bin
	@cp $(BUILD_WRKSRC)/pforth_standalone $(STAGEDIR)$(PREFIX)/bin/pforth_standalone
	@printf '%s\n' \
	    '#!/bin/sh' \
	    'bindir=$$(dirname "$$0")' \
	    'prefix=$$(cd "$$bindir/.." && pwd)' \
	    'case "$$1" in' \
	    '-d*|-i) exec "$$prefix/libexec/$(PORTNAME)/pforth-bin" "$$@" ;;' \
	    '*) exec "$$prefix/libexec/$(PORTNAME)/pforth-bin" -d "$$prefix/share/$(PORTNAME)/pforth.dic" "$$@" ;;' \
	    'esac' \
	    > $(STAGEDIR)$(PREFIX)/bin/pforth
	@chmod 755 $(STAGEDIR)$(PREFIX)/bin/pforth
	@if [ -f $(BUILD_WRKSRC)/pforth.dic ]; then \
	    cp $(BUILD_WRKSRC)/pforth.dic $(STAGEDIR)$(PREFIX)/share/$(PORTNAME)/pforth.dic; \
	fi
	@cp -R $(WRKSRC)/fth $(STAGEDIR)$(PREFIX)/share/$(PORTNAME)
	@cp $(WRKSRC)/README.md $(STAGEDIR)$(PREFIX)/share/doc/$(PORTNAME)
	@cp $(WRKSRC)/RELEASES.md $(STAGEDIR)$(PREFIX)/share/doc/$(PORTNAME)
	@cp $(WRKSRC)/license.txt $(STAGEDIR)$(PREFIX)/share/doc/$(PORTNAME)

include $(PORTSDIR)/Mk/linux.port.post.mk
