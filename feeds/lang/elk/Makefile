# Ports collection:     elk
# Date created:         2025/09/20
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= elk
DISTVERSION		= 3.99.8
CATEGORIES		= lang
MASTER_SITES		= https://sam.zoy.org/$(PORTNAME)/
DIST_SUBDIR		= $(PORTNAME)

# SOURCE_MODE=dist selects the release; SOURCE_MODE=git selects this revision.
# Keep the legacy default until project policy selects a mode explicitly.
USE_SCM			?= git
SCM_REPO_URL		= https://gitea.scheme.org/conservatory/elk.git
SCM_DETACH		= 5ba5ffc44da4943139fa27879df2aa748abf1c71
SCM_REPO_PREINIT_CMD	= .no-preinit

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
