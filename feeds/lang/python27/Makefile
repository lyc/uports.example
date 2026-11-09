# Ports collection:     python27
# Versioned native interpreter for historical build systems.

PORTNAME		= python27
DISTVERSION		= 2.7.18
DISTNAME		= Python-$(DISTVERSION)
CATEGORIES		= lang
MASTER_SITES		= https://www.python.org/ftp/python/$(DISTVERSION)/
DIST_SUBDIR		= python27

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= Python 2.7 interpreter for historical build tools
LICENSE			= PSFL
LICENSE_FILE		= $(WRKSRC)/LICENSE

USES			= tar:xz
GNU_CONFIGURE		= yes
CFLAGS			+= -std=gnu89
CONFIGURE_ARGS		+= --disable-shared --without-ensurepip
INSTALL_TARGET		= altinstall

ifneq ($(filter git,$(SOURCE_MODE) $(USE_SCM)),)
$(error Python27 requires release sources; use SOURCE_MODE=dist)
endif
ifneq ($(CROSS_COMPILE),)
$(error Python27 is a native build interpreter; unset CROSS_COMPILE)
endif

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../../../make/uports
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= post-install

# altinstall versions the interpreter, but still installs bare helper names.
post-install:
	@set -e; for name in 2to3 idle pydoc smtpd.py; do	       \
	    rm -f "$(STAGEDIR)$(PREFIX)/bin/$$name";		       \
	done

include $(PORTSDIR)/Mk/linux.port.post.mk
