# Ports collection      quickjs
# Date created:         2023/06/10
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= quickjs
DISTVERSION 		= 2021-03-27
CATEGORIES 		= lang
MASTER_SITES 		= https://bellard.org/quickjs/

MAINTAINER 		= max_lee@pegatroncorp.com
COMMENT			= QuickJS is a small and embedded Javascript engine.

LICENSE			= MIT

USES			+= tar:xz

ifneq ($(PREFIX),)
MAKE_ENV		+= prefix=$(PREFIX)
endif

#
#
#

MASTERDIR 		= $(CURDIR)
PORTSDIR 		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= post-patch pre-install post-install

post-patch:
	@$(SED) -e 's|%PREFIX%|$(PREFIX)|g ; s|%VERSION%|$(DISTVERSION)|g' \
		$(FILESDIR)/$(PORTNAME).pc.in > $(WRKDIR)/$(PORTNAME).pc
	@$(SED) -e 's|%PREFIX%|$(PREFIX)|g ; s|%VERSION%|$(DISTVERSION)|g' \
		$(FILESDIR)/$(PORTNAME).lto.pc.in > $(WRKDIR)/$(PORTNAME).lto.pc

pre-install:
	@mkdir -p $(addprefix $(STAGEDIR)$(PREFIX)/,lib/pkgconfig)

post-install:
	@cp $(WRKDIR)/$(PORTNAME).pc $(STAGEDIR)$(PREFIX)/lib/pkgconfig
	@cp $(WRKDIR)/$(PORTNAME).lto.pc $(STAGEDIR)$(PREFIX)/lib/pkgconfig

include $(PORTSDIR)/Mk/linux.port.post.mk
