# Ports collection:     pixman
# Native pixel manipulation library for QEMU.

PORTNAME		= pixman
DISTVERSION		= 0.34.0
CATEGORIES		= x11
MASTER_SITES		= https://www.cairographics.org/releases/
DIST_SUBDIR		= pixman

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= Pixel manipulation library
LICENSE			= MIT
LICENSE_FILE		= $(WRKSRC)/COPYING

USES			= libtool
GNU_CONFIGURE		= yes
CONFIGURE_ARGS		+= --disable-gtk --disable-libpng

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../../../make/uports
include $(PORTSDIR)/Mk/linux.port.mk
