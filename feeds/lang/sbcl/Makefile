# Ports collection      sbcl
# Date created:         2025/11/03
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= sbcl
DISTVERSION 		= 2.5.7
CATEGORIES 		= lang
DISTVERSIONSUFFIX	= -source
PORTEPOCH		= 1
MASTER_SITES		= https://sourceforge.net/projects/$(PORTNAME)/files/$(PORTNAME)/$(DISTVERSION)/
WRKSRC			= $(WRKDIR)/$(PORTNAME)-$(DISTVERSION)

USE_SCM			?= git
ifeq ($(USE_SCM),git)
SCM_TIMEOUT		?= 60s
SCM_PROTOCOL		= https://
MASTER_SITES		= git.code.sf.net/p/
MASTER_SITE_SUBDIR	= sbcl
SCM_DEST		= $(PORTNAME)
SCM_DETACH		= $(PORTNAME)-$(DISTVERSION)
WRKSRC			= $(WRKDIR)/$(PORTNAME)
endif

MAINTAINER 		= yowching.lee@gmail.com
COMMENT			= Common Lisp development system derived from the CMU CL system

LICENSE			= BSD2CLAUSE PD
LICENSE_COMB		= dual
LICENSE_FILE		= $(WRKSRC)/COPYING

LIB_DEPENDS		= libgmp.so:math/gmp				\
			  libmpfr.so:math/mpfr

# A source build requires a working Common Lisp implementation.  Prefer the
# reproducible uports bootstrap unless the caller explicitly selects a host
# implementation with WITH_SBCL_BOOTSTRAP=no and SBCL_XC_HOST.
WITH_SBCL_BOOTSTRAP	?= yes
ifeq ($(WITH_SBCL_BOOTSTRAP),yes)
BUILD_DEPENDS		+= sbcl-bootstrap>=0:lang/sbcl-bootstrap
endif

USES			= tar:bzip2

#EXTRACT_TRANSFORM	= /$(PORTNAME)-$(DISTVERSION)/$(PORTNAME)/g

SBCL_BOOTSTRAP_HOST	= $(DESTDIR)$(PREFIX)/bin/sbcl-bootstrap --no-userinit --no-sysinit
ifeq ($(WITH_SBCL_BOOTSTRAP),yes)
SBCL_XC_HOST		?= $(SBCL_BOOTSTRAP_HOST)
else ifneq ($(wildcard $(DESTDIR)$(PREFIX)/bin/sbcl-bootstrap),)
SBCL_XC_HOST		?= $(SBCL_BOOTSTRAP_HOST)
else
SBCL_XC_HOST		?= sbcl --no-userinit --no-sysinit
endif
SBCL_BUILD_ARGS		?= --prefix=$(PREFIX)
SBCL_INSTALL_ARGS	?= --prefix=$(PREFIX)

MAKE_ENV		+= GNUMAKE="$(MAKE)"				\
			   SBCL_HOME=					\
			   SBCL_XC_HOST="$(SBCL_XC_HOST)"

#
#
#

MASTERDIR 		= $(CURDIR)
PORTSDIR 		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= do-build do-install

do-build:
	@(cd $(WRKSRC) &&						\
	    $(SETENV) $(MAKE_ENV) $(SH) make.sh $(SBCL_BUILD_ARGS)	\
	        --xc-host="$(SBCL_XC_HOST)")

do-install:
	@(cd $(WRKSRC) &&						\
	    $(SETENV) $(MAKE_ENV) BUILD_ROOT=$(STAGEDIR)		\
	        $(SH) install.sh $(SBCL_INSTALL_ARGS))

include $(PORTSDIR)/Mk/linux.port.post.mk
