# Ports collection:     patchelf
# Native ELF editor used when preparing toolchain distributions.

PORTNAME		= patchelf
DISTVERSION		= 0.18.0
CATEGORIES		= sysutils
MASTER_SITES		= https://github.com/NixOS/patchelf/releases/download/$(DISTVERSION)/
DIST_SUBDIR		= patchelf

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= Modify ELF interpreters and runtime library paths
LICENSE			= GPLv3+
LICENSE_FILE		= $(WRKSRC)/COPYING

# The release asset includes configure and needs no Autotools bootstrap.
USES			= tar:bz2
GNU_CONFIGURE		= yes

ifneq ($(strip $(CROSS_COMPILE) $(CONFIGURE_TARGET)),)
$(error Patchelf builds natively; unset CROSS_COMPILE and CONFIGURE_TARGET)
endif
ifneq ($(filter git,$(SOURCE_MODE) $(USE_SCM)),)
$(error Patchelf currently requires release sources; use SOURCE_MODE=dist)
endif

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../../../make/uports
include $(PORTSDIR)/Mk/linux.port.mk
