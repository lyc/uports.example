# Ports collection:     mpc
# Date created:         3/March/2013
# Whom:                 Max Lee

PORTNAME		= mpc
PORTVERSION		= 1.4.1
PORTREVISION		= 0
CATEGORIES		= devel
MASTER_SITES		= GNU/mpc

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= Gnu Mpc is a C library for the arithmetic of complex numbers with arbitrarily high precision and correct rounding of the result.

LICENSE			= LGPLv3

FLAVORS			= default mit2018
FLAVOR			?= $(firstword $(FLAVORS))
mit2018_PKGNAMESUFFIX	= -mit2018
mit2018_PLIST		= $(MASTERDIR)/pkg-plist.mit2018.$(OPSYS)-$(ARCH)
DISTINFO_FILE		= $(MASTERDIR)/distinfo.$(PORTVERSION)

GNU_CONFIGURE		= yes
CONFIGURE_SCRIPT	= configure

ifeq ($(FLAVOR),mit2018)
PORTVERSION		= 0.9
MASTER_SITES		= https://www.multiprecision.org/downloads/
LICENSE			= LGPL21+
LICENSE_FILE		= $(WRKSRC)/COPYING.LIB
PREFIX			?= /i386-jos-elf
LIB_DEPENDS		= libgmp.so:math/gmp@mit2018	       \
			  libmpfr.so:math/mpfr@mit2018
EXTRACT_SUFX		= .tar.gz
USES			+= libtool:keepla
CFLAGS			+= -std=gnu89
# The release skips EGREP discovery when CC is supplied by the framework.
CONFIGURE_ENV		+= EGREP="$(GREP) -E"
PATCHDIR		= $(MASTERDIR)/files/mit2018
# Use the selected native providers, not consumer PREFIX guesses.
UPORTS_LIB_DEPENDS_USES_UPORTS = no
MAKE_ARGS		+= MAKEINFO=true
ALL_TARGET		= all check

ifneq ($(filter git,$(SOURCE_MODE) $(USE_SCM)),)
$(error MPC mit2018 requires the release archive; use SOURCE_MODE=dist)
endif
ifneq ($(CROSS_COMPILE),)
$(error MPC mit2018 is a native compiler dependency; unset CROSS_COMPILE)
endif
else
LIB_DEPENDS		= libgmp.so:math/gmp				\
			  libmpfr.so:math/mpfr

# Git remains the default; SOURCE_MODE=dist selects the release tarball.
USE_SCM			?= git
SCM_REPO_URL		= https://gitlab.inria.fr/mpc/mpc.git
SCM_DETACH		= $(PORTVERSION)

USES			+= autoreconf tar:xz libtool

CONFIGURE_ENV		+= CPPFLAGS=-fexceptions
CONFIGURE_ARGS		+= --with-gnu-ld				\
			  --with-gmp=$(DESTDIR)$(PREFIX)		\
			  --with-mpfr=$(DESTDIR)$(PREFIX)

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
	    -e 's|^SUBDIRS = src tests doc tools|SUBDIRS = src tests tools|' \
	    -e 's| install-nobase_dist_docDATA||'			\
	    -e 's| install-dist_docDATA||'				\
	    $(CONFIGURE_WRKSRC)/Makefile
endif
endif

include $(PORTSDIR)/Mk/linux.port.post.mk
