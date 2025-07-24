# Prts collection:	libyang
# Date created:		10/Jun/2022
# Whom:                 yowching.lee@gmail.com

PORTNAME		= libyang
DISTVERSIONPREFIX	= v
DISTVERSION		= 3.7.8
CATEGORIES		= net
MASTER_SITES 		= $(MASTER_SITE_GITHUB)
MASTER_SITE_SUBDIR	= CESNET/$(PORTNAME)
DISTNAME		= v$(DISTVERSION)
WRKSRC			= $(WRKDIR)/$(PORTNAME)-$(PORTVERSION)
DIST_SUBDIR		= $(PORTNAME)

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= YANG data modeling language library

LICENSE			= BSD3CLAUSE
LICENSE_FILE		= $(WRKSRC)/LICENSE

LIB_DEPENDS		= libpcre2-8.so:devel/pcre2
#TEST_DEPENDS		= cmocka>0:sysutils/cmocka

#USES			= cmake compiler:c++11-lang cpe pkgconfig
USES			= cmake cpe pkgconfig

CPE_VENDOR		= cesnet
#USE_LDCONFIG		= yes

#USE_GITHUB		= yes
#GH_ACCOUNT		= CESNET

CMAKE_ARGS		+= -DCMAKE_INSTALL_LIBDIR=lib			\
			   -DPLUGINS_DIR=$(PREFIX)/lib/libyang

#TEST_TARGET		= test
#
#OPTIONS_DEFINE		= TEST
#
#TEST_CMAKE_BOOL	= ENABLE_BUILD_TESTS

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.mk
