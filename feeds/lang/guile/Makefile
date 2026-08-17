# Ports collection:     guile
# Date created:         2025/09/20
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= guile
DISTVERSION		= 3.0.10
CATEGORIES		= lang
MASTER_SITES		= $(MASTER_SITE_GNU)
MASTER_SITE_SUBDIR	= $(PORTNAME)
DIST_SUBDIR		= $(PORTNAME)

# Set USE_SCM= to use the GNU release tarball from MASTER_SITES.
USE_SCM			?= git
ifeq ($(USE_SCM),git)
SCM_PROTOCOL		= https://
MASTER_SITES		= git.savannah.gnu.org/
MASTER_SITE_SUBDIR	= git
SCM_DEST		= $(DISTNAME)
SCM_DETACH		= v$(DISTVERSION)
endif

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= GNU extension language and Scheme implementation

LICENSE			= LGPL3+
LICENSE_FILE		= $(WRKSRC)/LICENSE

BUILD_DEPENDS		= pkg-config:devel/pkg-config
LIB_DEPENDS		= libreadline.so:devel/readline			\
			  libunistring.so:devel/libunistring		\
			  libgmp.so:math/gmp				\
			  libffi.so:devel/libffi			\
			  libgc.so:devel/boehm-gc

USES			= libtool tar:xz

GNU_CONFIGURE		= yes

# pkg-config is a build-host tool.  The project wrapper exports the selected
# host-tools instance; the fallback also keeps direct port builds usable.
PKG_CONFIG		?= pkg-config
CONFIGURE_ENV		+= PKG_CONFIG=$(PKG_CONFIG) \
			   PKG_CONFIG_LIBDIR=$(DESTDIR)$(PREFIX)/lib/pkgconfig:$(DESTDIR)$(PREFIX)/lib64/pkgconfig \
			   PKG_CONFIG_SYSROOT_DIR=$(DESTDIR)
# Darwin's linker drops Guile's hidden internal symbols during libtool's
# partial-link aggregation on arm64.  Keep them visible until the final link.
ifeq ($(shell uname -s),Darwin)
CFLAGS			+= -fvisibility=default
endif
ifeq ($(shell uname -s),Linux)
GUILE_BUILD_LIBRARY_PATH = $(DESTDIR)$(PREFIX)/lib:$(DESTDIR)$(PREFIX)/lib64
CONFIGURE_ENV		+= LD_LIBRARY_PATH=$(GUILE_BUILD_LIBRARY_PATH)
MAKE_ENV		+= LD_LIBRARY_PATH=$(GUILE_BUILD_LIBRARY_PATH)
endif
CONFIGURE_ARGS		+= --disable-error-on-warning			\
			   --disable-lto				\
			   --disable-rpath
INSTALL_TARGET		= install-strip

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= post-install post-stage

post-install:
	@$(SED) -e 's|%VERSION%|$(DISTVERSION)|g' \
		$(FILESDIR)/guile-3.0.pc.in \
		> $(STAGEDIR)$(PREFIX)/lib/pkgconfig/guile-3.0.pc

post-stage:
ifeq ($(OPSYS),darwin)
	@find $(STAGEDIR)$(PREFIX) -type f | while read f; do		\
	    otool -l "$$f" >/dev/null 2>&1 || continue;			\
	    for rpath in $(addprefix $(DESTDIR)$(PREFIX)/,$(libdirs)); do \
	        otool -l "$$f" |					\
	            grep -Fq "path $$rpath (offset" || continue;	\
	        install_name_tool -delete_rpath "$$rpath" "$$f";	\
	    done;							\
	done
endif

include $(PORTSDIR)/Mk/linux.port.post.mk
