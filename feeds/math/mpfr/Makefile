# Ports collection:     mpfr
# Date created:         3/March/2013
# Whom:                 Max Lee

PORTNAME		= mpfr
PORTVERSION		= 4.2.2
PORTREVISION		= 0
CATEGORIES		= devel
MASTER_SITES		= https://www.mpfr.org/mpfr-$(PORTVERSION)/

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= The MPFR library is a C library for multiple-precision floating-point computations with correct rounding.

LICENSE			= LGPLv3

FLAVORS			= default mit2018
FLAVOR			?= $(firstword $(FLAVORS))
mit2018_PKGNAMESUFFIX	= -mit2018
mit2018_PLIST		= $(MASTERDIR)/pkg-plist.mit2018.$(OPSYS)-$(ARCH)
DISTINFO_FILE		= $(MASTERDIR)/distinfo.$(PORTVERSION)

GNU_CONFIGURE		= yes
CONFIGURE_SCRIPT	= configure

ifeq ($(FLAVOR),mit2018)
PORTVERSION		= 3.1.2
PREFIX			?= /i386-jos-elf
LIB_DEPENDS		= libgmp.so:math/gmp@mit2018
USES			+= tar:bz2 libtool:keepla
CFLAGS			+= -std=gnu89
PATCHDIR		= $(MASTERDIR)/files/mit2018
# Use the selected native GMP provider's paths, not consumer PREFIX guesses.
UPORTS_LIB_DEPENDS_USES_UPORTS = no
MAKE_ARGS		+= MAKEINFO=true
ALL_TARGET		= all check

ifneq ($(filter git,$(SOURCE_MODE) $(USE_SCM)),)
$(error MPFR mit2018 requires the release archive; use SOURCE_MODE=dist)
endif
ifneq ($(CROSS_COMPILE),)
$(error MPFR mit2018 is a native compiler dependency; unset CROSS_COMPILE)
endif
else
LIB_DEPENDS		= libgmp.so:math/gmp

# Git remains the default; SOURCE_MODE=dist selects the release tarball.
USE_SCM			?= git
SCM_REPO_URL		= https://gitlab.inria.fr/mpfr/mpfr.git
SCM_DETACH		= $(PORTVERSION)

USES			+= tar:xz libtool

CONFIGURE_ENV		+= CPPFLAGS=-fexceptions
CONFIGURE_ARGS		+= --with-gnu-ld				\
			  --with-gmp=$(DESTDIR)$(PREFIX)

WITH_DOCUMENT		?= no

endif

# Libraries use build/host configuration, including CROSS_COMPILE when set.
# CONFIGURE_TARGET selects compiler output, not this library architecture.
# Keep PREFIX and work layout under project control.
ifneq ($(CONFIGURE_TARGET),)
override CONFIGURE_TARGET	:=
endif

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../../../make/uports
include $(PORTSDIR)/Mk/linux.port.pre.mk

ifeq ($(FLAVOR),mit2018)
override_targets	+= post-install

post-install:
	@rm -f "$(STAGEDIR)$(PREFIX)/share/info/dir"
else
ifneq ($(WITH_DOCUMENT),yes)
override_targets	+= post-configure

post-configure:
	@$(SED) -i.bak							\
	    -e 's|^SUBDIRS = doc src tests tune tools/bench|SUBDIRS = src tests tune tools/bench|' \
	    -e 's| install-nobase_dist_docDATA||'			\
	    $(CONFIGURE_WRKSRC)/Makefile
endif
endif

include $(PORTSDIR)/Mk/linux.port.post.mk
