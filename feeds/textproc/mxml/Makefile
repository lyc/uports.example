# Ports collection:      mxml
# Date created:         28/Aug/2026
# Whom:                 yowching.lee@gmail.com

PORTNAME		= mxml
DISTVERSIONPREFIX	= v
DISTVERSION		= 4.0.5
CATEGORIES		= textproc
USE_GITHUB		= yes
GH_ACCOUNT		= michaelrsweet
DIST_SUBDIR		= $(PORTNAME)

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= Tiny XML library

LICENSE			= APACHE20
LICENSE_FILE		= $(WRKSRC)/LICENSE

GNU_CONFIGURE		= yes
CONFIGURE_ARGS		+= --enable-shared

ifeq ($(shell uname -s),Darwin)
MXML_ARCH		= $(if $(filter aarch64,$(ARCH)),arm64,$(ARCH))
CONFIGURE_ENV		+= OPTIM="-arch $(MXML_ARCH)"
endif

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.mk
