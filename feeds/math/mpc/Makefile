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

LIB_DEPENDS		= libgmp.so:math/gmp				\
			  libmpfr.so:math/mpfr

# Git remains the default; SOURCE_MODE=dist selects the release tarball.
USE_SCM			?= git
SCM_REPO_URL		= https://gitlab.inria.fr/mpc/mpc.git
SCM_DETACH		= $(PORTVERSION)

USES			+= autoreconf tar:xz libtool

GNU_CONFIGURE		= yes
CONFIGURE_SCRIPT	= configure
CONFIGURE_ENV		+= CPPFLAGS=-fexceptions
CONFIGURE_ARGS		+= --with-gnu-ld				\
			  --with-gmp=$(DESTDIR)$(PREFIX)		\
			  --with-mpfr=$(DESTDIR)$(PREFIX)

WITH_DOCUMENT		?= no

ifneq ($(WITH_DOCUMENT),yes)
override_targets	+= post-configure

post-configure:
	@$(SED) -i.bak							\
	    -e 's|^SUBDIRS = src tests doc tools|SUBDIRS = src tests tools|' \
	    -e 's| install-nobase_dist_docDATA||'			\
	    -e 's| install-dist_docDATA||'				\
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
PORTSDIR		= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.mk
