# Ports collection      tcc
# Date created:         2025/10/23
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= tinycc
DISTVERSION 		= 0.9.28rc.20260714
CATEGORIES 		= lang
TINYCC_COMMIT		= d9d02c56401e43be43760b63f7d82f771a7ed1f6
USE_GITHUB		= yes
GH_ACCOUNT		= TinyCC
GH_TAGNAME		= $(TINYCC_COMMIT)
DIST_SUBDIR		= $(PORTNAME)

# Set SOURCE_MODE=dist to use the pinned upstream snapshot tarball.
USE_SCM			?= git
SCM_REPO_URL		= https://github.com/TinyCC/$(PORTNAME).git
SCM_DETACH		= $(TINYCC_COMMIT)
SCM_RETRIEVE_TIMEOUT	= 300

MAINTAINER 		= yowching.lee@gmail.com
COMMENT			= Small C compiler with an integrated assembler and linker

LICENSE			= LGPL21
LICENSE_FILE		= $(WRKSRC)/COPYING

GNU_CONFIGURE		= yes
MAKE_ENV		+= ZERO_AR_DATE=1
# Keep the compiler identity identical for the Git checkout and commit archive.
MAKE_ARGS		+= GITHASH="2026-07-14 HEAD@d9d02c56"

#
#
#

MASTERDIR 		= $(CURDIR)
PORTSDIR 		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.mk
