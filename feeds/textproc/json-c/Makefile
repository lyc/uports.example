# Prts collection:	json-c
# Date created:		14/Feb/2006
# Whom:                 colin@askey.com.tw

PORTNAME		= json-c
DISTVERSION		= 0.19
DISTVERSIONSUFFIX	= -20260627
CATEGORIES		= textproc
USE_GITHUB		= yes
GH_ACCOUNT		= json-c
GH_TAGNAME		= $(PORTNAME)-$(DISTVERSIONFULL)

MAINTAINER		= dtchang@askey.com.tw
COMMENT			= A JSON implementation in C

LICENSE			= MIT

USES			= cmake

# Preserve the existing Git-mode patch policy; dist mode applies files/series.
NO_SCM_PATCH_APPLY	= yes

CMAKE_ARGS		+= -DCMAKE_INSTALL_LIBDIR=lib

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.mk
