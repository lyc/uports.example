# Ports collection:	libuuid
# Date created:		2026/06/06
# Whom:			Max Lee <yowching.lee@gmail.com>

PORTNAME		= libuuid
DISTVERSION		= 2.41.4
CATEGORIES		= devel
MASTER_SITES		= https://www.kernel.org/pub/linux/utils/util-linux/v2.41/
DISTNAME		= util-linux-$(DISTVERSION)

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= UUID generation library from util-linux

LICENSE			= BSD3CLAUSE
LICENSE_FILE		= $(WRKSRC)/Documentation/licenses/COPYING.BSD-3-Clause

USES			= libtool pkgconfig tar:xz

GNU_CONFIGURE		= yes
CONFIGURE_ARGS		+= --disable-all-programs			\
			   --disable-libblkid				\
			   --disable-libfdisk				\
			   --disable-libmount				\
			   --disable-libsmartcols			\
			   --enable-libuuid				\
			   --disable-makeinstall-chown			\
			   --disable-nls				\
			   --without-python				\
			   --without-systemd				\
			   --without-systemdsystemunitdir

override_targets	+= post-install

post-install:
	@$(LN) -sf uuid/uuid.h $(STAGEDIR)$(PREFIX)/include/uuid.h

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.mk
