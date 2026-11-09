# Ports collection:     gcc
# Native compiler executables for explicitly selected target profiles.

PORTNAME		= gcc
CATEGORIES		= lang
MASTER_SITES		= GNU/gcc/gcc-$(DISTVERSION)
DIST_SUBDIR		= gcc

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= GNU C compiler and freestanding target libgcc
LICENSE			= GPLv3+ GCC-RLE
LICENSE_FILE		= $(WRKSRC)/COPYING3

ifneq ($(filter git,$(SOURCE_MODE) $(USE_SCM)),)
$(error GCC currently supports release sources; use SOURCE_MODE=dist)
endif

# Further native/RISC-V/bootstrap configurations require their own acceptance.
FLAVORS			= mit2018 jos12
FLAVOR			?= $(firstword $(FLAVORS))
mit2018_PKGNAMESUFFIX	= -mit2018
jos12_PKGNAMESUFFIX	= -jos12
mit2018_PLIST		= $(MASTERDIR)/pkg-plist.mit2018.$(OPSYS)-$(ARCH)
jos12_PLIST		= $(MASTERDIR)/pkg-plist.jos12.$(OPSYS)-$(ARCH)
DISTINFO_FILE		= $(MASTERDIR)/distinfo.$(DISTVERSION)
CONFIGURE_TARGET	= i386-jos-elf
ifneq ($(CONFIGURE_TARGET),i386-jos-elf)
$(error GCC JOS Flavors require CONFIGURE_TARGET=i386-jos-elf)
endif
PACKAGE_BUILD_ID	= $(CONFIGURE_TARGET)-c-freestanding

ifeq ($(FLAVOR),mit2018)
DISTVERSION		= 4.6.4
DISTNAME		= gcc-core-$(DISTVERSION)
PREFIX			?= /$(CONFIGURE_TARGET)
USES			= tar:bz2
CFLAGS			+= -std=gnu89
BUILD_DEPENDS		= i386-jos-elf-as:devel/binutils@mit2018
LIB_DEPENDS		= libgmp.so:math/gmp@mit2018		       \
			  libmpfr.so:math/mpfr@mit2018		       \
			  libmpc.so:math/mpc@mit2018
CONFIGURE_ARGS		+= --enable-languages=c --disable-werror       \
			   --disable-libssp --disable-libmudflap       \
			   --with-newlib --without-headers
else
DISTVERSION		= 12.5.0
PREFIX			?= /i386-jos-elf-gcc12
USES			= tar:xz
BUILD_DEPENDS		= i386-jos-elf-as:devel/binutils@jos12
LIB_DEPENDS		= libgmp.so:math/gmp@default		       \
			  libmpfr.so:math/mpfr@default		       \
			  libmpc.so:math/mpc@default
CONFIGURE_ARGS		+= --enable-languages=c --disable-bootstrap    \
			   --without-headers --with-newlib	       \
			   --disable-multilib --disable-shared	       \
			   --disable-threads --disable-nls	       \
			   --disable-werror --disable-libssp	       \
			   --disable-default-pie --disable-default-ssp \
			   --without-isl --without-zstd
endif

# Native dependency paths come from the selected providers, not target PREFIX.
UPORTS_LIB_DEPENDS_USES_UPORTS	= no

GNU_CONFIGURE		= yes
WRKSRC			= $(WRKDIR)/gcc-$(DISTVERSION)
CONFIGURE_WRKSRC	= $(WRKDIR)/build
BUILD_WRKSRC		= $(CONFIGURE_WRKSRC)
INSTALL_WRKSRC		= $(CONFIGURE_WRKSRC)
CONFIGURE_SCRIPT	= ../$(notdir $(WRKSRC))/configure
gcc-build-tools		= $(DESTDIR)$(PREFIX)/$(CONFIGURE_TARGET)/bin
CONFIGURE_ARGS		+= --with-build-time-tools=$(gcc-build-tools)

# Do not pass native dependency include/library flags to target libgcc.
CONFIGURE_ENV		+= CFLAGS_FOR_TARGET="-O2 -ffreestanding"      \
			   CPPFLAGS_FOR_TARGET= LDFLAGS_FOR_TARGET=
MAKE_ENV		+= PATH="$(DESTDIR)$(PREFIX)/bin:$$PATH"
ALL_TARGET		= all-gcc

# Static native math archives avoid embedding development-library runpaths.
gcc-native-math-libs	= $(shell cat "$(WRKDIR)/native-math-libs" 2>/dev/null)
MAKE_ARGS		+= MAKEINFO=true			       \
			   HOST_GMPLIBS="$(gcc-native-math-libs)"      \
			   GMPLIBS="$(gcc-native-math-libs)"	       \
			   CFLAGS_FOR_TARGET="-O2 -ffreestanding"      \
			   CPPFLAGS_FOR_TARGET= LDFLAGS_FOR_TARGET=

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../../../make/uports
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= pre-configure post-build do-install	       \
			   post-install do-clean

pre-configure:
	@$(MKDIR) "$(CONFIGURE_WRKSRC)"
	@set -e; libraries=;					       \
	for name in mpc mpfr gmp; do				       \
	    archive=;						       \
	    for flag in $(LDFLAGS); do				       \
	        case "$$flag" in				       \
	            -L*) candidate="$${flag#-L}/lib$$name.a";	       \
	                if [ -f "$$candidate" ]; then		       \
	                    archive=$$candidate; break;		       \
	                fi;;					       \
	        esac;						       \
	    done;						       \
	    if [ -z "$$archive" ]; then				       \
	        printf 'Missing native static lib%s.a in provider paths\n' \
	          "$$name" >&2;					       \
	        exit 1;						       \
	    fi;							       \
	    libraries="$$libraries $$archive";			       \
	done;							       \
	printf '%s\n' "$$libraries" > "$(WRKDIR)/native-math-libs"
	@set -e; for tool in as ld ar nm ranlib strip; do	       \
	    test -x "$(gcc-build-tools)/$$tool";		       \
	done

# all-gcc must finish before the newly built compiler builds target libgcc.
post-build:
	@(cd "$(BUILD_WRKSRC)" && $(SETENV) $(MAKE_ENV)		       \
	  $(MAKE) $(MAKE_FLAGS) $(MAKEFILE) $(MAKE_ARGS) all-target-libgcc)

do-install:
	@set -e; for target in install-gcc install-target-libgcc; do   \
	    (cd "$(INSTALL_WRKSRC)" && $(SETENV) $(MAKE_ENV)	       \
	      $(MAKE) $(MAKE_FLAGS) $(MAKEFILE) DESTDIR="$(STAGEDIR)"  \
	      $(MAKE_ARGS) "$$target");				       \
	done

post-install:
	@rm -f "$(STAGEDIR)$(PREFIX)/share/info/dir"

do-clean:
	@if [ -f "$(BUILD_WRKSRC)/Makefile" ]; then		       \
	    $(MAKE) -C "$(BUILD_WRKSRC)" clean;			       \
	fi
	@rm -f "$(BUILD_COOKIE)" "$(INSTALL_COOKIE)" "$(PACKAGE_COOKIE)"

include $(PORTSDIR)/Mk/linux.port.post.mk

# Let each GCC subconfigure select its own compiler's preprocessor.
# Inheriting the host CPP makes target header checks inspect host headers.
MAKE_ENV		+= CPP=
