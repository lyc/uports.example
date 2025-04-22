# Prts collection:	libffcall
# Date created:		2025/04/02
# Whom:                 yowching.lee@gmail.com

PORTNAME		= libffcall
DISTVERSION		= 2.5
CATEGORIES		= devel
MASTER_SITES 		= $(MASTER_SITE_GNU)
MASTER_SITE_SUBDIR	= $(PORTNAME)

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= Foreigh function call library

LICENSE			= GPLv2+
LICENSE_FILE		= $(WRKSRC)/COPYING

USES			= libtool

GNU_CONFIGURE		= yes
CONFIGURE_ARGS		+= --enable-shared

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk
override_targets	+= post-patch post-install

post-patch:
	@$(SED) -e 's|%PREFIX%|$(PREFIX)|g ; s|%VERSION%|$(PORTVERSION)|g' \
		$(FILESDIR)/$(PORTNAME).pc.in > $(WRKDIR)/$(PORTNAME).pc

post-install:
	@mkdir -p $(addprefix $(STAGEDIR)$(PREFIX)/,lib/pkgconfig)
	@cp $(WRKDIR)/$(PORTNAME).pc $(STAGEDIR)$(PREFIX)/lib/pkgconfig

include $(PORTSDIR)/Mk/linux.port.post.mk
