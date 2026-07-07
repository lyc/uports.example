# Ports collection:	libsoundio
# Date created:		2026/06/27
# Whom:			Max Lee <yowching.lee@gmail.com>

PORTNAME		= libsoundio
DISTVERSION		= 2.0.0
CATEGORIES		= audio

USE_SCM			= git
SCM_PROTOCOL		= https://
MASTER_SITES		= github.com/
MASTER_SITE_SUBDIR	= andrewrk
SCM_DEST		= $(PORTNAME)-$(DISTVERSION)
SCM_DETACH		= $(DISTVERSION)
WRKSRC			= $(WRKDIR)/$(SCM_DEST)

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= Cross-platform real-time audio input and output library

LICENSE			= MIT
LICENSE_FILE		= $(WRKSRC)/LICENSE

USES			= cmake

CMAKE_ARGS		+= -DCMAKE_INSTALL_LIBDIR=lib			\
			   -DBUILD_EXAMPLE_PROGRAMS=OFF			\
			   -DBUILD_TESTS=OFF

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
