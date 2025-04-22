# Prts collection:	libffi
# Date created:		2013/03/04
# Whom:                 yowching.lee@gmail.com

PORTNAME		= libffi
DISTVERSIONPREFIX	= v
DISTVERSION		= 3.4.6
CATEGORIES		= devel
MASTER_SITES 		= $(MASTER_SITE_GITHUB)
MASTER_SITE_SUBDIR	= libffi/$(PORTNAME)
DISTNAME		= v$(DISTVERSION)
WRKSRC			= $(WRKDIR)/$(PORTNAME)-$(PORTVERSION)
DIST_SUBDIR		= $(PORTNAME)

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= A Rortable Foreigh Function Interface Library

LICENSE			= BSD3CLAUSE
LICENSE_FILE		= $(WRKSRC)/LICENSE

LIB_DEPENDS		= libncurses.so:devel/ncurses			\
			  libreadline.so:devel/readline

USES			= autoreconf libtool

GNU_CONFIGURE		= yes
CONFIGURE_ARGS		+= --disable-multi-os-directory

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk
override_targets	+= post-configure

post-configure:
# NOTE:
#   macOS may installed with case insensitive file system and
#   INSTALL file (created from autoreconf) will conflict "install" make target
ifeq ($(OPSYS),darwin)
	@rm -fr $(WRKSRC)/INSTALL
endif

include $(PORTSDIR)/Mk/linux.port.post.mk
