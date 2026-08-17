# Ports collection:     elk
# Date created:         2025/09/20
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= elk
DISTVERSION		= 3.99.8
CATEGORIES		= lang
MASTER_SITES		= https://sam.zoy.org/$(PORTNAME)/
DIST_SUBDIR		= $(PORTNAME)

# Set USE_SCM= to use the official release tarball from MASTER_SITES.
USE_SCM			?= git
ifeq ($(USE_SCM),git)
SCM_PROTOCOL		= https://
MASTER_SITES		= gitea.scheme.org/
MASTER_SITE_SUBDIR	= conservatory
SCM_DEST		= $(DISTNAME)
SCM_DETACH		= 5ba5ffc44da4943139fa27879df2aa748abf1c71
SCM_REPO_PREINIT_CMD	= .no-preinit
SCM_RETRIEVE_TIMEOUT	= 300
endif

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= Embeddable Scheme implementation for C and C++ applications

LICENSE			= ELK
LICENSE_FILE		= $(WRKSRC)/COPYING

USES			= autoreconf libtool:keepla tar:bzip2

GNU_CONFIGURE		= yes
CFLAGS			+= -std=gnu89
INSTALL_TARGET		= install-strip

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.mk
