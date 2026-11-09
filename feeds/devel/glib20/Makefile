# Ports collection:     glib20
# Native GLib support for the historical MIT QEMU build.

PORTNAME		= glib
DISTVERSION		= 2.56.4
CATEGORIES		= devel
MASTER_SITES		= https://download.gnome.org/sources/glib/2.56/
DIST_SUBDIR		= glib20

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= GLib core application support libraries
LICENSE			= LGPLv2.1+
LICENSE_FILE		= $(WRKSRC)/COPYING

USES			= tar:xz libtool pkgconfig
GNU_CONFIGURE		= yes
BUILD_DEPENDS		= python2.7:lang/python27 msgfmt:devel/gettext
RUN_DEPENDS		= python2.7:lang/python27
LIB_DEPENDS		= libffi.so:devel/libffi libz.so:archivers/zlib
UPORTS_LIB_DEPENDS_USES_UPORTS	= no
GLIB_PYTHON		?= python2.7
CONFIGURE_ARGS		+= --with-python=$(GLIB_PYTHON)		       \
			   --with-pcre=internal --disable-libmount     \
			   --disable-libelf --disable-selinux	       \
			   --disable-fam --disable-gtk-doc	       \
			   --disable-man --disable-installed-tests     \
			   --disable-always-build-tests

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../../../make/uports
include $(PORTSDIR)/Mk/linux.port.mk
