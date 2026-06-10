# Ports collection      gdbm
# Date created:         2024/08/06
# Whom:                 Max Lee <max_lee@pegatroncorp.com>

PORTNAME		= gdbm
DISTVERSION 		= 1.24
CATEGORIES 		= databases
MASTER_SITES 		= $(MASTER_SITE_GNU)
MASTER_SITE_SUBDIR	= $(PORTNAME)

MAINTAINER 		= max_lee@pegatroncorp.com
COMMENT			= GNU database manager

LICENSE			= GPL-3+

LIB_DEPENDS		= libncurses.so.6:devel/ncurses			\
			  libreadline.so:devel/readline

USES			= libtool pkgconfig

#USE_LDCONFIG 		= yes
#USE_AUTOTOOLS 		= yes
GNU_CONFIGURE 		= yes
CONFIGURE_ARGS		+= --enable-libgdbm-compat


#
#
#

MASTERDIR 		= $(CURDIR)
PORTSDIR 		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.mk
