# Ports collection:     binutils
# Target configurations for host-running GNU binary utilities.

PORTNAME		= binutils
CATEGORIES		= devel
MASTER_SITES		= GNU/binutils
DIST_SUBDIR		= binutils

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= GNU assembler, linker and binary utilities
LICENSE			= GPLv3+
LICENSE_FILE		= $(WRKSRC)/COPYING3

# The release contains generated configure/parser/documentation files.
# Git source preparation is outside this initial release-based profile.
ifneq ($(filter git,$(SOURCE_MODE) $(USE_SCM)),)
$(error Binutils currently supports release sources; use SOURCE_MODE=dist)
endif

FLAVORS			= default mit2018 jos12 mit2026
FLAVOR			?= $(firstword $(FLAVORS))
mit2018_PKGNAMESUFFIX	= -mit2018
jos12_PKGNAMESUFFIX	= -jos12
mit2026_PKGNAMESUFFIX	= -mit2026
mit2018_PLIST		= $(MASTERDIR)/pkg-plist.mit2018.$(OPSYS)-$(ARCH)
jos12_PLIST		= $(MASTERDIR)/pkg-plist.jos12.$(OPSYS)-$(ARCH)
mit2026_PLIST		= $(MASTERDIR)/pkg-plist.mit2026.$(OPSYS)-$(ARCH)

ifeq ($(FLAVOR),mit2018)
DISTVERSION		= 2.21.1
BINUTILS_TARGET		= i386-jos-elf
BINUTILS_PREFIX		= /i386-jos-elf
else ifeq ($(FLAVOR),jos12)
DISTVERSION		= 2.46.1
BINUTILS_TARGET		= i386-jos-elf
BINUTILS_PREFIX		= /i386-jos-elf-gcc12
else ifeq ($(FLAVOR),mit2026)
DISTVERSION		= 2.47
BINUTILS_TARGET		= riscv64-unknown-elf
BINUTILS_PREFIX		= /riscv64-unknown-elf
else
DISTVERSION		= 2.47
BINUTILS_TARGET		=
BINUTILS_PREFIX		= /usr/local
endif
DISTINFO_FILE		= $(MASTERDIR)/distinfo.$(DISTVERSION)

# All executables run natively; only the MIT Flavors select an output target.
CONFIGURE_TARGET	= $(BINUTILS_TARGET)
ifneq ($(CONFIGURE_TARGET),$(BINUTILS_TARGET))
$(error FLAVOR=$(FLAVOR) requires CONFIGURE_TARGET=$(BINUTILS_TARGET))
endif
PREFIX			?= $(BINUTILS_PREFIX)
PACKAGE_BUILD_ID	= $(or $(CONFIGURE_TARGET),native)

ifeq ($(FLAVOR),mit2018)
USES			= tar:bz2
CFLAGS			+= -std=gnu89
CONFIGURE_ARGS		+= --disable-werror
else
USES			= tar:xz
CONFIGURE_ARGS		+= --disable-werror --disable-nls	       \
			   --disable-shared --enable-static	       \
			   --disable-multilib --disable-gprofng	       \
			   --disable-libctf --without-debuginfod       \
			   --without-zstd --without-system-zlib
endif
GNU_CONFIGURE		= yes
CONFIGURE_WRKSRC	= $(WRKDIR)/build
BUILD_WRKSRC		= $(CONFIGURE_WRKSRC)
INSTALL_WRKSRC		= $(CONFIGURE_WRKSRC)
CONFIGURE_SCRIPT	= ../$(notdir $(WRKSRC))/configure

# Release documentation is shipped prebuilt; do not require host Texinfo.
MAKE_ARGS		+= MAKEINFO=true

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../../../make/uports
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= pre-configure do-clean

pre-configure:
	@$(MKDIR) "$(CONFIGURE_WRKSRC)"

ifeq ($(FLAVOR),mit2018)
override_targets	+= post-install

post-install:
	@rm -f "$(STAGEDIR)$(PREFIX)/share/info/dir"
endif

# The configured Makefile is in the separate build tree.
do-clean:
	@if [ -f "$(BUILD_WRKSRC)/Makefile" ]; then		       \
	    $(MAKE) -C "$(BUILD_WRKSRC)" clean;			       \
	fi
	@rm -f "$(BUILD_COOKIE)" "$(INSTALL_COOKIE)" "$(PACKAGE_COOKIE)"

include $(PORTSDIR)/Mk/linux.port.post.mk
