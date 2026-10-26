# Prts collection:	libyang
# Date created:		10/Jun/2022
# Whom:                 yowching.lee@gmail.com

PORTNAME		= libyang
DISTVERSIONPREFIX	= v
DISTVERSION		= 3.7.8
CATEGORIES		= net
USE_GITHUB		= yes
GH_ACCOUNT		= CESNET
DIST_SUBDIR		= $(PORTNAME)

# SOURCE_MODE can override the default Git source selection.
USE_SCM			?= git
SCM_REPO_URL		= https://github.com/$(GH_ACCOUNT)/$(PORTNAME).git
SCM_DETACH		= $(DISTVERSIONFULL)

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
include $(PORTSDIR)/Mk/linux.port.pre.mk

# CMake caches the source path; keep archive and Git builds separate.
CONFIGURE_WRKSRC	= $(WRKDIR)/.build-$(notdir $(WRKSRC))

include $(PORTSDIR)/Mk/linux.port.post.mk
