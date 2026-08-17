# Ports collection      gambit
# Date created:         2025/08/25
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= gambit
DISTVERSIONPREFIX	= v
DISTVERSION 		= 4.9.7
CATEGORIES 		= lang
MASTER_SITES 		= https://gambitscheme.org/$(DISTVERSION)/
DISTVERSIONFULL		= $(DISTVERSIONPREFIX)$(subst .,_,$(DISTVERSION))
DIST_SUBDIR		= $(PORTNAME)

# Set USE_SCM= to use the official release tarball from MASTER_SITES.
USE_SCM			?= git
ifeq ($(USE_SCM),git)
SCM_PROTOCOL		= https://
MASTER_SITES		= github.com/
MASTER_SITE_SUBDIR	= gambit
SCM_DEST		= $(DISTNAME)
SCM_DETACH		= v$(DISTVERSION)
SCM_RETRIEVE_TIMEOUT	= 300
endif

GAMBIT_RELEASE_EPOCH	= 1752404565

MAINTAINER 		= yowching.lee@gmail.com
COMMENT			= Gambit programming system where the compiler generates portable C code

LICENSE			= APACHE20 LGPL21
LICENSE_COMB		= dual

USES			= tar:tgz

NO_CHECK_CERTIFICATE	= yes
GNU_CONFIGURE		= yes
CONFIGURE_ARGS		+= --enable-gcc-opts				\
			   --includedir=$(PREFIX)/include/gambit	\
			   --libdir=$(PREFIX)/lib/gambit		\
			   --enable-compiler-name=gsc-gambit		\
			   --enable-interpreter-name=gsi-gambit
MAKEFILE		= makefile

#
#
#

MASTERDIR 		= $(CURDIR)
PORTSDIR 		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets 	+= post-patch

post-patch:
ifneq ($(USE_SCM),git)
	@GIT_AUTHOR_DATE=@$(GAMBIT_RELEASE_EPOCH)			\
	    GIT_COMMITTER_DATE=@$(GAMBIT_RELEASE_EPOCH)			\
	    $(GIT) -C $(WRKSRC) commit --amend --no-edit			\
	    --date=@$(GAMBIT_RELEASE_EPOCH) >/dev/null
endif
	@$(GIT) -C $(WRKSRC) tag -f v$(DISTVERSION)

include $(PORTSDIR)/Mk/linux.port.post.mk
