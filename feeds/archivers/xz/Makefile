# Ports collection:	xz
# Date created:		2026/06/06
# Whom:			Max Lee <yowching.lee@gmail.com>

PORTNAME		= xz
DISTVERSION		= 5.8.3
CATEGORIES		= archivers
MASTER_SITES		= https://github.com/tukaani-project/xz/releases/download/v$(DISTVERSION)/

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= XZ and LZMA compression utilities

LICENSE			= Public-Domain LGPL21 GPLv2 GPLv3
LICENSE_FILE		= $(WRKSRC)/COPYING

USES			= libtool pkgconfig tar:xz

GNU_CONFIGURE		= yes
CONFIGURE_ARGS		+= --enable-shared				\
			   --disable-doc				\
			   --disable-nls

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.mk
