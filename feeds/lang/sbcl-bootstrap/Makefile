# Ports collection      sbcl-bootstrap
# Date created:         2026/05/29
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= sbcl
PKGNAMESUFFIX		= -bootstrap
CATEGORIES 		= lang

MAINTAINER 		= yowching.lee@gmail.com
COMMENT			= Prebuilt SBCL used only as an SBCL bootstrap host

LICENSE			= BSD2CLAUSE PD
LICENSE_COMB		= dual
LICENSE_FILE		= $(WRKSRC)/COPYING

SBCL_BOOTSTRAP_OPSYS	?= $(shell uname -s | tr '[:upper:]' '[:lower:]')
SBCL_BOOTSTRAP_ARCH	?= $(shell uname -m)
ifeq ($(SBCL_BOOTSTRAP_OPSYS),darwin)
ifeq ($(SBCL_BOOTSTRAP_ARCH),arm64)
SBCL_BOOTSTRAP_ARCH	:= aarch64
endif
endif

ifeq ($(SBCL_BOOTSTRAP_OPSYS),linux)
ifeq ($(SBCL_BOOTSTRAP_ARCH),x86_64)
SBCL_BOOTSTRAP_VERSION	= 2.6.3
SBCL_BOOTSTRAP_PLATFORM	= x86-64-linux
else ifeq ($(SBCL_BOOTSTRAP_ARCH),aarch64)
SBCL_BOOTSTRAP_VERSION	= 1.4.2
SBCL_BOOTSTRAP_PLATFORM	= arm64-linux
else ifeq ($(SBCL_BOOTSTRAP_ARCH),arm64)
SBCL_BOOTSTRAP_VERSION	= 1.4.2
SBCL_BOOTSTRAP_PLATFORM	= arm64-linux
else ifeq ($(SBCL_BOOTSTRAP_ARCH),i386)
SBCL_BOOTSTRAP_VERSION	= 1.4.3
SBCL_BOOTSTRAP_PLATFORM	= x86-linux
else ifeq ($(SBCL_BOOTSTRAP_ARCH),i686)
SBCL_BOOTSTRAP_VERSION	= 1.4.3
SBCL_BOOTSTRAP_PLATFORM	= x86-linux
endif
else ifeq ($(SBCL_BOOTSTRAP_OPSYS),darwin)
ifeq ($(SBCL_BOOTSTRAP_ARCH),x86_64)
SBCL_BOOTSTRAP_VERSION	= 2.2.9
SBCL_BOOTSTRAP_PLATFORM	= x86-64-darwin
else ifeq ($(SBCL_BOOTSTRAP_ARCH),aarch64)
SBCL_BOOTSTRAP_VERSION	= 2.4.0
SBCL_BOOTSTRAP_PLATFORM	= arm64-darwin
endif
endif

ifeq ($(SBCL_BOOTSTRAP_VERSION),)
$(error unsupported SBCL bootstrap platform: $(SBCL_BOOTSTRAP_OPSYS)/$(SBCL_BOOTSTRAP_ARCH))
endif

DISTVERSION 		= $(SBCL_BOOTSTRAP_VERSION)
MASTER_SITES		= https://downloads.sourceforge.net/project/$(PORTNAME)/$(PORTNAME)/$(DISTVERSION)/
DISTNAME		= $(PORTNAME)-$(DISTVERSION)-$(SBCL_BOOTSTRAP_PLATFORM)
DISTFILES		= $(DISTNAME)-binary$(EXTRACT_SUFX)
DIST_SUBDIR		= $(PORTNAME)-bootstrap

USES			= tar:bzip2

SBCL_BOOTSTRAP_ROOT	= $(PREFIX)/libexec/sbcl-bootstrap

#
#
#

MASTERDIR 		= $(CURDIR)
PORTSDIR 		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= do-build do-install

do-build:

do-install:
	@/bin/mkdir -p $(STAGEDIR)$(PREFIX)/bin
	@/bin/mkdir -p $(STAGEDIR)$(SBCL_BOOTSTRAP_ROOT)/bin
	@/bin/mkdir -p $(STAGEDIR)$(SBCL_BOOTSTRAP_ROOT)/lib/sbcl
	@/bin/mkdir -p $(STAGEDIR)$(PREFIX)/share/doc/sbcl-bootstrap
	@/usr/bin/install -c -m 755 $(WRKSRC)/src/runtime/sbcl $(STAGEDIR)$(SBCL_BOOTSTRAP_ROOT)/bin/sbcl
	@/usr/bin/install -c -m 644 $(WRKSRC)/output/sbcl.core $(STAGEDIR)$(SBCL_BOOTSTRAP_ROOT)/lib/sbcl/sbcl.core
	@/usr/bin/install -c -m 644 $(WRKSRC)/COPYING $(STAGEDIR)$(PREFIX)/share/doc/sbcl-bootstrap/COPYING
	@/usr/bin/install -c -m 644 $(WRKSRC)/CREDITS $(STAGEDIR)$(PREFIX)/share/doc/sbcl-bootstrap/CREDITS
	@{ \
		echo '#!/bin/sh'; \
		echo 'bindir=$$(CDPATH= cd -- "$$(dirname -- "$$0")" && pwd) || exit 1'; \
		echo 'root=$$(CDPATH= cd -- "$$bindir/../libexec/sbcl-bootstrap" && pwd) || exit 1'; \
		echo 'if [ -z "$$SBCL_HOME" ]; then'; \
		echo '    SBCL_HOME=$$root/lib/sbcl'; \
		echo '    export SBCL_HOME'; \
		echo 'fi'; \
		echo 'exec "$$root/bin/sbcl" "$$@"'; \
	} > $(STAGEDIR)$(PREFIX)/bin/sbcl-bootstrap
	@chmod 755 $(STAGEDIR)$(PREFIX)/bin/sbcl-bootstrap

include $(PORTSDIR)/Mk/linux.port.post.mk
