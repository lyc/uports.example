# Ports collection:     openssl
# Date created:         2026/05/21
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= openssl
DISTVERSION		= 3.5.6
CATEGORIES		= security
MASTER_SITES		= https://www.openssl.org/source/

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= TLS/SSL and cryptography toolkit

LICENSE			= Apache-2.0
LICENSE_FILE		= $(WRKSRC)/LICENSE.txt

USES			= git:ownership

LIB_DEPENDS		= libz.so:archivers/zlib

# Set USE_SCM= to use the release tarball from MASTER_SITES.
USE_SCM			?= git
ifeq ($(USE_SCM),git)
SCM_PROTOCOL		= https://
MASTER_SITES		= github.com/
MASTER_SITE_SUBDIR	= openssl
SCM_DEST		= $(DISTNAME)
SCM_DETACH		= $(PORTNAME)-$(DISTVERSION)
endif

ifneq ($(CROSS_COMPILE),)
  CONFIGURE_SCRIPT	:= Configure
  CONFIGURE_ENV		+= CROSS_COMPILE=
  ifneq ($(filter arm,$(subst -, ,$(CROSS_COMPILE))),)
    CONFIGURE_ARGS	+= linux-armv4
  endif
  ifneq ($(filter mips,$(subst -, ,$(CROSS_COMPILE))),)
    CONFIGURE_ARGS	+= linux-mipsel
  endif
  DONT_USE_HOST_TRIPLET	= yes
else
  CONFIGURE_SCRIPT	:= config
  ifeq ($(shell uname -s),Darwin)
      CONFIGURE_SCRIPT	:= Configure
    ifeq ($(shell uname -m),x86_64)
      CONFIGURE_ARGS	+= darwin64-x86_64-cc
    endif
    ifeq ($(shell uname -m),arm64)
      CONFIGURE_ARGS	+= darwin64-arm64-cc
    endif
  endif
endif

GNU_CONFIGURE		= yes
CONFIGURE_ARGS		+= --openssldir=$(PREFIX)/lib/ssl		\
			   --libdir=lib				\
			   no-asm					\
			   shared					\
			   zlib
MAKE_ARGS		+= INSTALL_PREFIX="$(STAGEDIR)"

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.mk
