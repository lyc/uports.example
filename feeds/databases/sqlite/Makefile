# Ports collection      sqlite
# Date created:         2026/06/05
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= sqlite
DISTVERSION		= 3.53.2
CATEGORIES		= databases
MASTER_SITES		= https://sqlite.org/2026/
SQLITE_DISTVERSION	= 3530200
DISTNAME		= sqlite-autoconf-$(SQLITE_DISTVERSION)
WRKSRC			= $(WRKDIR)/$(if $(filter git,$(SOURCE_MODE_EFFECTIVE)),$(SCM_DEST),$(DISTNAME))
DIST_SUBDIR		= $(PORTNAME)

# Set USE_SCM=git to use the GitHub mirror of the official Fossil repository.
USE_SCM			?=
SCM_REPO_URL		= https://github.com/sqlite/sqlite.git
SCM_DEST		= $(PORTNAME)-$(DISTVERSION)
SCM_DETACH		= version-$(DISTVERSION)

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= SQL database engine in a C library

LICENSE			= Public-Domain

USES			= libtool pkgconfig

GNU_CONFIGURE		= yes
CONFIGURE_ARGS		+= --enable-shared				\
			   --disable-readline

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.mk
