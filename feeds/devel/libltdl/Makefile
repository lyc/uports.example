# Ports collection:	libltdl
# Date created:		21/Apr/2025
# Whom:			Max Lee <yowching.lee@gmail.com>

PORTNAME		= libltdl
PORTREVISION		= 0
DISTNAME		= libtool-$(DISTVERSIONFULL)
WRKSRC_SUBDIR		= libltdl

COMMENT			= System independent dlopen wrapper

include                 $(PORTSDIR)/devel/libtool/Makefile.common

LICENSE			= LGPL21
LICENSE_FILE		= $(WRKSRC)/COPYING

USES			+= libtool:keepla

GNU_CONFIGURE		= yes
CONFIGURE_ARGS		= --enable-ltdl-install
INSTALL_TARGET		= install-strip
USE_LDCONFIG		= yes

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
