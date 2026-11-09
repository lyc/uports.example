# Ports collection:     gettext
# Native catalog tools required by GLib's release configure checks.

PORTNAME		= gettext
DISTVERSION		= 0.22.5
CATEGORIES		= devel
MASTER_SITES		= GNU/gettext
DIST_SUBDIR		= gettext

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= GNU message catalog tools and runtime support
LICENSE			= GPLv3+ LGPLv2.1+
LICENSE_FILE		= $(WRKSRC)/COPYING

USES			= tar:xz libtool
GNU_CONFIGURE		= yes
CONFIGURE_ARGS		+= --disable-shared --disable-java	       \
			   --disable-libasprintf --disable-openmp      \
			   --with-included-libunistring --without-emacs
MAKE_ARGS		+= MAKEINFO=true

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../../../make/uports
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= do-build post-install

# Internal libtextstyle header installation writes into the build tree.
# A deployment DESTDIR must not redirect those intermediate files.
do-build:
	@(cd "$(BUILD_WRKSRC)" && $(SETENV) $(MAKE_ENV)		       \
	  $(MAKE) $(MAKE_FLAGS) $(MAKEFILE) $(MAKE_ARGS)	       \
	  DESTDIR= $(ALL_TARGET))

post-install:
	@rm -f "$(STAGEDIR)$(PREFIX)/share/info/dir"

include $(PORTSDIR)/Mk/linux.port.post.mk
