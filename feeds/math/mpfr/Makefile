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

LIB_DEPENDS		= libgmp.so:math/gmp

# Git remains the default; SOURCE_MODE=dist selects the release tarball.
USE_SCM			?= git
SCM_REPO_URL		= https://gitlab.inria.fr/mpfr/mpfr.git
SCM_DETACH		= $(PORTVERSION)

USES			+= tar:xz libtool

GNU_CONFIGURE		= yes
CONFIGURE_SCRIPT	= configure
CONFIGURE_ENV		+= CPPFLAGS=-fexceptions
CONFIGURE_ARGS		+= --with-gnu-ld				\
			  --with-gmp=$(DESTDIR)$(PREFIX)

WITH_DOCUMENT		?= no

ifneq ($(WITH_DOCUMENT),yes)
override_targets	+= post-configure

post-configure:
	@$(SED) -i.bak							\
	    -e 's|^SUBDIRS = doc src tests tune tools/bench|SUBDIRS = src tests tune tools/bench|' \
	    -e 's| install-nobase_dist_docDATA||'			\
	    $(CONFIGURE_WRKSRC)/Makefile
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
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.mk
