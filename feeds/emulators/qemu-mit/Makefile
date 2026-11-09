# Ports collection:     qemu-mit
# MIT's historical QEMU fork, separate from upstream emulator recipes.

PORTNAME		= qemu-mit
DISTVERSION		= 2.3.0.20180907
CATEGORIES		= emulators
USE_GITHUB		= yes
GH_ACCOUNT		= mit-pdos
GH_PROJECT		= 6.828-qemu
GH_TAGNAME		= d531b1b1d6b7696dfd9695c1d560e3df53e615c5
DIST_SUBDIR		= qemu-mit

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= MIT QEMU fork with JOS debugging support
LICENSE			= GPLv2 LGPLv2.1 BSD3CLAUSE MIT
LICENSE_FILE		= $(WRKSRC)/LICENSE

FLAVORS			= mit2018
FLAVOR			?= $(firstword $(FLAVORS))
mit2018_PKGNAMESUFFIX	= -mit2018
mit2018_PLIST		= $(MASTERDIR)/pkg-plist.mit2018.$(OPSYS)-$(ARCH)
PREFIX			?= /i386-jos-elf

SCM_REPO_URL		= https://github.com/mit-pdos/6.828-qemu.git
SCM_DETACH		= $(GH_TAGNAME)

BUILD_DEPENDS		= python2.7:lang/python27
LIB_DEPENDS		= libglib-2.0.so:devel/glib20		       \
			  libpixman-1.so:x11/pixman libz.so:archivers/zlib
UPORTS_LIB_DEPENDS_USES_UPORTS	= no
USES			= pkgconfig
QEMU_PYTHON		?= python2.7
# Native dependencies are installed below the same deployment root.
ifneq ($(DESTDIR),)
CONFIGURE_ENV		+= PKG_CONFIG_SYSROOT_DIR="$(DESTDIR)"
endif

ifneq ($(CROSS_COMPILE),)
$(error MIT QEMU runs natively; unset CROSS_COMPILE)
endif
ifneq ($(CONFIGURE_TARGET),)
$(error MIT QEMU selects emulation with target-list, not CONFIGURE_TARGET)
endif

HAS_CONFIGURE		= yes
CONFIGURE_WRKSRC	= $(WRKDIR)/build
BUILD_WRKSRC		= $(CONFIGURE_WRKSRC)
INSTALL_WRKSRC		= $(CONFIGURE_WRKSRC)
CONFIGURE_SCRIPT	= ../$(notdir $(WRKSRC))/configure
CONFIGURE_ARGS		+= --prefix=$(PREFIX)			       \
			   --extra-cflags="$(CPPFLAGS) $(CFLAGS)"      \
			   --target-list=i386-softmmu,x86_64-softmmu   \
			   --python=$(QEMU_PYTHON)			       \
			   --with-system-pixman			       \
			   --disable-kvm --disable-werror	       \
			   --disable-sdl --disable-gtk --disable-vnc   \
			   --disable-curses --disable-docs	       \
			   --disable-fdt --disable-tools	       \
			   --disable-guest-agent

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../../../make/uports
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= pre-configure

pre-configure:
	@$(MKDIR) "$(CONFIGURE_WRKSRC)"

include $(PORTSDIR)/Mk/linux.port.post.mk
