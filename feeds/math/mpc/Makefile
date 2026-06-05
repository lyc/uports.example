# Ports collection:     mpc
# Date created:         3/March/2013
# Whom:                 Max Lee

PORTNAME		= mpc
PORTVERSION		= 1.4.1
PORTREVISION		= 0
CATEGORIES		= devel
MASTER_SITES		= http://ftp.gnu.org/gnu/mpc/

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= Gnu Mpc is a C library for the arithmetic of complex numbers with arbitrarily high precision and correct rounding of the result.

LICENSE			= LGPLv3

LIB_DEPENDS		= libgmp.so:math/gmp				\
			  libmpfr.so:math/mpfr

# Set USE_SCM= to use the release tarball from MASTER_SITES.
USE_SCM			?= git
ifeq ($(USE_SCM),git)
SCM_PROTOCOL		= https://
MASTER_SITES		= gitlab.inria.fr/
MASTER_SITE_SUBDIR	= mpc
SCM_DEST		= $(DISTNAME)
SCM_DETACH		= $(PORTVERSION)
endif

# FIXME: should put this setting into project template makefile.
# Assume we are trying to build a toolchain which config as $(CONFIGURE_TARGET),
# it's a good idea to install gpm/mpfr/mpc ... outside the folder $(PREFIX)
# toolchain will be installed.
ifneq ($(CONFIGURE_TARGET),)
ifneq ($(PREFIX),)
override PREFIX		:= $(PREFIX)/tools
endif
endif

USES			+= autoreconf tar:xz libtool

GNU_CONFIGURE		= yes
ifneq ($(CONFIGURE_TARGET),)
WRKSRC_SUBDIR		:= build-$(word 1,$(subst -, ,$(CONFIGURE_TARGET)))
CONFIGURE_SCRIPT	= ../configure
else
CONFIGURE_SCRIPT	= configure
endif
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

# NOTE: there is no need to set --target=... during configure because
# gmp/mpfr/mpc ... was only built as host tool to support "binutils"
# and "gcc" building, so we need to unset $(CONFIGURE_TARGET) if it's set.
ifneq ($(CONFIGURE_TARGET),)
override CONFIGURE_TARGET	:=
endif

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.mk
