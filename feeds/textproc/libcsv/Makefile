# Prts collection:	libcsv
# Date created:		24/Jul/2025
# Whom:                 yowching.lee@gmail.com

PORTNAME		= libcsv
DISTVERSION		= 3.0.3
CATEGORIES		= textproc
MASTER_SITES		= https://sourceforge.net/projects/$(PORTNAME)/files/$(PORTNAME)/$(PORTNAME)-$(DISTVERSION)/

MAINTAINER		= yowching.lee@mail.com
COMMENT			= Small, simple and fast CSV parser

LICENSE			= LGPL21

USE			= libtool

GNU_CONFIGURE		= yes


#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk
override_targets	+= post-patch post-install

post-patch:
	@$(SED) -e 's|%PREFIX%|$(PREFIX)|g ; s|%VERSION%|$(DESTVERSION)|g' \
		$(FILESDIR)/$(PORTNAME).pc.in > $(WRKDIR)/$(PORTNAME).pc

post-install:
	@mkdir -p $(addprefix $(STAGEDIR)$(PREFIX)/,lib/pkgconfig)
	@cp $(WRKDIR)/$(PORTNAME).pc $(STAGEDIR)$(PREFIX)/lib/pkgconfig

include $(PORTSDIR)/Mk/linux.port.post.mk
