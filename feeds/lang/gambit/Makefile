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

# Keep the official release archive independent of Git source selection.
USE_SCM			?= git
SCM_REPO_URL		= https://github.com/gambit/gambit.git
SCM_DETACH		= v$(DISTVERSION)
SCM_RETRIEVE_TIMEOUT	= 300

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
ifeq ($(SOURCE_MODE_EFFECTIVE),dist)
	@GIT_AUTHOR_DATE=@$(GAMBIT_RELEASE_EPOCH)			\
	    GIT_COMMITTER_DATE=@$(GAMBIT_RELEASE_EPOCH)			\
	    $(GIT) -C $(WRKSRC) commit --amend --no-edit			\
	    --date=@$(GAMBIT_RELEASE_EPOCH) >/dev/null
	@$(GIT) -C $(WRKSRC) tag -f v$(DISTVERSION)
endif

include $(PORTSDIR)/Mk/linux.port.post.mk
