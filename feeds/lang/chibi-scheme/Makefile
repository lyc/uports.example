# Ports collection      chibi-scheme
# Date created:         2025/10/28
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= chibi-scheme
DISTVERSION 		= 0.12
CATEGORIES 		= lang
MASTER_SITES 		= https://github.com/ashinn/$(PORTNAME)/archive/refs/tags/
DISTFILES		= $(DISTVERSIONFULL)$(EXTRACT_SUFX)
WRKSRC			= $(WRKDIR)/$(PORTNAME)-$(DISTVERSION)
DIST_SUBDIR		= $(PORTNAME)

USE_SCM			?= git
ifeq ($(USE_SCM),git)
SCM_PROTOCOL		= https://
MASTER_SITES		= github.com/
MASTER_SITE_SUBDIR	= ashinn
SCM_DEST		= $(DISTNAME)
SCM_DETACH		= $(DISTVERSIONFULL)
endif

MAINTAINER 		= yowching.lee@gmail.com
COMMENT			= Minimal Scheme Implementation for use as an Extension Langage

LICENSE			= BSD-3-Clause
LICENSE_FILE		= $(WRKSRC)/COPYING

LIB_DEPENDS		=

MAKE_ARGS		+= PREFIX=$(PREFIX)

#
#
#

MASTERDIR 		= $(CURDIR)
PORTSDIR 		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= post-stage

post-stage:
ifeq ($(OPSYS),darwin)
	@for f in $(STAGEDIR)$(PREFIX)/bin/chibi-scheme			\
	    $(STAGEDIR)$(PREFIX)/bin/chibi-doc				\
	    $(STAGEDIR)$(PREFIX)/bin/chibi-ffi				\
	    $(STAGEDIR)$(PREFIX)/bin/snow-chibi; do			\
	    install_name_tool -add_rpath @loader_path/../lib "$$f"	\
	        2>/dev/null || true;					\
	done
	@find $(STAGEDIR)$(PREFIX)/lib/chibi -type f -name '*.dylib' |	\
	while read f; do						\
	    rel=`echo "$$f" | $(SED) -e 's|^$(STAGEDIR)$(PREFIX)/lib/chibi/||'`; \
	    depth=`echo "$$rel" | awk -F/ '{ print NF - 1 }'`;		\
	    rpath='@loader_path';					\
	    i=0;							\
	    while [ $$i -le $$depth ]; do				\
	        rpath="$$rpath/..";					\
	        i=`expr $$i + 1`;					\
	    done;							\
	    install_name_tool -add_rpath "$$rpath" "$$f"		\
	        2>/dev/null || true;					\
	done
endif

include $(PORTSDIR)/Mk/linux.port.post.mk
