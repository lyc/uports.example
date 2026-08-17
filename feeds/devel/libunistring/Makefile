# Ports collection:      libunistring

PORTNAME		= libunistring
DISTVERSION		= 1.4.2
CATEGORIES		= devel
MASTER_SITES		= $(MASTER_SITE_GNU)
MASTER_SITE_SUBDIR	= $(PORTNAME)
DIST_SUBDIR		= $(PORTNAME)

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= Unicode string library

LICENSE			= LGPL3+ GPL2+
LICENSE_COMB		= dual
LICENSE_FILE		= $(WRKSRC)/COPYING

LIB_DEPENDS		= libiconv.so:converters/libiconv

USES			= libtool tar:xz

GNU_CONFIGURE		= yes
INSTALL_TARGET		= install-strip

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= post-patch post-install

post-patch:
	@$(SED) -e 's|%PREFIX%|$(PREFIX)|g ; s|%VERSION%|$(DISTVERSION)|g' \
		$(FILESDIR)/$(PORTNAME).pc.in > $(WRKDIR)/$(PORTNAME).pc

post-install:
	@mkdir -p $(STAGEDIR)$(PREFIX)/lib/pkgconfig
	@cp $(WRKDIR)/$(PORTNAME).pc $(STAGEDIR)$(PREFIX)/lib/pkgconfig

include $(PORTSDIR)/Mk/linux.port.post.mk
