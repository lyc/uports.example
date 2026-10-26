# Ports collection      quickjs
# Date created:         2023/06/10
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= quickjs
DISTVERSION 		= 2025-09-13
PORTREVISION		= 2
CATEGORIES 		= lang
MASTER_SITES 		= https://bellard.org/quickjs/
DISTNAME		= $(PORTNAME)-$(DISTVERSION)-$(PORTREVISION)
DISTNAME_EXTRA		= $(PORTNAME)-extras-$(DISTVERSION)
DISTFILES		= $(addsuffix					\
			    $(EXTRACT_SUFX),$(DISTNAME) $(DISTNAME_EXTRA))
WRKSRC			= $(WRKDIR)/$(PORTNAME)-$(DISTVERSION)

USE_SCM			?= git
SCM_REPO_URL		= https://github.com/bellard/$(PORTNAME).git
# Preserve the existing unpinned Git default; release archives are independent.

MAINTAINER 		= max_lee@pegatroncorp.com
COMMENT			= QuickJS is a small and embedded Javascript engine.

LICENSE			= MIT

USES			+= tar:xz

ifneq ($(PREFIX),)
MAKE_ENV		+= prefix=$(PREFIX)
endif
# Upstream Intl tests compare local Date values with fixed calendar strings.
# Run them in UTC so their result does not depend on the build host timezone.
MAKE_ENV		+= TZ=UTC
ALL_TARGET		= testall

#
#
#

MASTERDIR 		= $(CURDIR)
PORTSDIR 		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk

ifeq ($(SOURCE_MODE_EFFECTIVE),git)
WRKSRC			= $(WRKDIR)/$(PORTNAME)
endif

override_targets	+= post-patch pre-install post-install

post-patch:
	@$(SED) -e 's|%PREFIX%|$(PREFIX)|g ; s|%VERSION%|$(DISTVERSION)|g' \
		$(FILESDIR)/$(PORTNAME).pc.in > $(WRKDIR)/$(PORTNAME).pc
ifeq ($(SOURCE_MODE_EFFECTIVE),git)
	@$(MAKE) -C $(WRKSRC) test2-bootstrap
endif

pre-install:
	@mkdir -p $(addprefix $(STAGEDIR)$(PREFIX)/,lib/pkgconfig)

post-install:
	@cp $(WRKDIR)/$(PORTNAME).pc $(STAGEDIR)$(PREFIX)/lib/pkgconfig

include $(PORTSDIR)/Mk/linux.port.post.mk
