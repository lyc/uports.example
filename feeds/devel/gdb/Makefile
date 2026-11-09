# Ports collection:     gdb
# Native debugger for the historical JOS target.

PORTNAME		= gdb
DISTVERSION		= 7.3.1
CATEGORIES		= devel
MASTER_SITES		= GNU/gdb
DIST_SUBDIR		= gdb

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= GNU debugger for the historical JOS toolchain
LICENSE			= GPLv3+
LICENSE_FILE		= $(WRKSRC)/COPYING3

FLAVORS			= mit2018
FLAVOR			?= $(firstword $(FLAVORS))
mit2018_PKGNAMESUFFIX	= -mit2018
mit2018_PLIST		= $(MASTERDIR)/pkg-plist.mit2018.$(OPSYS)-$(ARCH)
DISTINFO_FILE		= $(MASTERDIR)/distinfo.$(DISTVERSION)
CONFIGURE_TARGET	= i386-jos-elf
PREFIX			?= /i386-jos-elf
PACKAGE_BUILD_ID	= i386-jos-elf-native-providers

ifneq ($(CONFIGURE_TARGET),i386-jos-elf)
$(error GDB mit2018 requires CONFIGURE_TARGET=i386-jos-elf)
endif
ifneq ($(CROSS_COMPILE),)
$(error GDB mit2018 runs natively; unset CROSS_COMPILE)
endif
ifneq ($(filter git,$(SOURCE_MODE) $(USE_SCM)),)
$(error GDB mit2018 requires release sources; use SOURCE_MODE=dist)
endif

USES			= tar:bz2
LIB_DEPENDS		= libncurses.so:devel/ncurses		       \
			  libz.so:archivers/zlib
# Use selected native providers, not Binutils' libiberty in the target prefix.
UPORTS_LIB_DEPENDS_USES_UPORTS	= no
GNU_CONFIGURE		= yes
CFLAGS			+= -std=gnu89
CONFIGURE_WRKSRC	= $(WRKDIR)/build
BUILD_WRKSRC		= $(CONFIGURE_WRKSRC)
INSTALL_WRKSRC		= $(CONFIGURE_WRKSRC)
CONFIGURE_SCRIPT	= ../$(notdir $(WRKSRC))/configure
CONFIGURE_ARGS		+= --program-prefix=i386-jos-elf-	       \
			   --disable-werror --without-python	       \
			   --with-curses --with-zlib=yes		       \
			   --without-system-readline --without-expat  \
			   --disable-nls
MAKE_ARGS		+= MAKEINFO=true

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../../../make/uports
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= pre-configure post-install

pre-configure:
	@$(MKDIR) "$(CONFIGURE_WRKSRC)"

post-install:
	@rm -f "$(STAGEDIR)$(PREFIX)/share/info/dir"
	@rm -f "$(STAGEDIR)$(PREFIX)/lib/libiberty.a"
	@set -e; for name in bfd configure standards; do	       \
	    rm -f "$(STAGEDIR)$(PREFIX)/share/info/$$name.info"*;      \
	done
	@set -e; if [ -d "$(STAGEDIR)$(PREFIX)/share/locale" ]; then   \
	    find "$(STAGEDIR)$(PREFIX)/share/locale" -type f	       \
	      \( -name bfd.mo -o -name opcodes.mo \) -exec rm -f {} +; \
	fi

include $(PORTSDIR)/Mk/linux.port.post.mk
