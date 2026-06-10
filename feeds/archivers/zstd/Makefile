# Ports collection:	zstd
# Date created:		2026/06/06
# Whom:			Max Lee <yowching.lee@gmail.com>

PORTNAME		= zstd
DISTVERSION		= 1.5.7
CATEGORIES		= archivers
MASTER_SITES		= https://github.com/facebook/zstd/releases/download/v$(DISTVERSION)/

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= Fast real-time compression algorithm

LICENSE			= BSD3CLAUSE GPLv2
LICENSE_FILE		= $(WRKSRC)/LICENSE

USES			=

MAKE_ARGS		+= PREFIX=$(PREFIX)				\
			   LIBDIR=$(PREFIX)/lib			\
			   MANDIR=$(PREFIX)/share/man

ALL_TARGET		= lib-release zstd-release

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.mk
