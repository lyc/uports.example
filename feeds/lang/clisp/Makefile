# Ports collection      clisp
# Date created:         2025/10/28
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= clisp
DISTVERSION 		= 2.49.95
CATEGORIES 		= lang
MASTER_SITES		= https://anduin.linuxfromscratch.org/BLFS/clisp/

#USE_SCM			?= git
ifeq ($(USE_SCM),git)
SCM_PROTOCOL		= https://
MASTER_SITES		= gitlab.com/
MASTER_SITE_SUBDIR	= gnu-clisp
SCM_DEST		= $(DISTNAME)
SCM_DETACH		= master
endif

MAINTAINER 		= yowching.lee@gmail.com
COMMENT			= GNU CLISP is a Common Lisp implementation.

LICENSE			= GPL-2
LICENSE_FILE		= $(WRKSRC)/GNU-GPL

WITH_READLINE		?= yes
WITH_GETTEXT		?= no

LIB_DEPENDS		= libavcall.so:devel/libffcall			\
			  libcallback.so:devel/libffcall
ifeq ($(WITH_READLINE),yes)
LIB_DEPENDS		+= libreadline.so:devel/readline
endif
ifneq ($(shell uname -s),Darwin)
LIB_DEPENDS		+= libiconv.so:converters/libiconv
endif

USES			= tar:xz libtool

CLISP_BUILDDIR		= $(WRKSRC)/build
BUILD_WRKSRC		= $(CLISP_BUILDDIR)
INSTALL_WRKSRC		= $(CLISP_BUILDDIR)
CLISP_LIBDIR		= $(PREFIX)/lib/$(PORTNAME)-2.49.95+
CLISP_BUILD_PREFIX	= $(DESTDIR)$(PREFIX)
CLISP_BUILD_INCLUDEPATH	= $(DESTDIR)$(PREFIX)/include
CLISP_BUILD_LIBPATH	= $(DESTDIR)$(PREFIX)/lib
CLISP_RUNTIME_INCLUDEPATH = $(PREFIX)/include
CLISP_RUNTIME_LIBPATH	= $(PREFIX)/lib

CONFIGURE_ARGS		+= --prefix=$(PREFIX)				\
			   --mandir=$(PREFIX)/share/man			\
			   --docdir=$(PREFIX)/share/doc/$(PORTNAME)	\
			   --with-libffcall-prefix=$(DESTDIR)$(PREFIX)	\
			   --with-dynamic-ffi				\
			   --ignore-absence-of-libsigsegv
ifeq ($(WITH_READLINE),yes)
CONFIGURE_ARGS		+= --with-readline				\
			   --with-libreadline-prefix=$(DESTDIR)$(PREFIX)
else
CONFIGURE_ARGS		+= --without-readline
endif
ifeq ($(WITH_GETTEXT),yes)
CONFIGURE_ARGS		+= --with-gettext
else
CONFIGURE_ARGS		+= --without-gettext
endif
ifneq ($(shell uname -s),Darwin)
CONFIGURE_ARGS		+= --with-libiconv-prefix=$(DESTDIR)$(PREFIX)
endif

#
#
#

MASTERDIR 		= $(CURDIR)
PORTSDIR 		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= do-configure pre-install post-install post-stage

do-configure:
	@(cd $(WRKSRC) &&						\
	    rm -fr $(CLISP_BUILDDIR) &&					\
	    $(SETENV) CC="$(CC)" CPP="$(CPP)" CXX="$(CXX)"		\
	        CFLAGS="$(CFLAGS)" CPPFLAGS="$(CPPFLAGS)"		\
	        CXXFLAGS="$(CXXFLAGS)" LDFLAGS="$(LDFLAGS)"		\
	        $(CONFIGURE_ENV) ./configure $(CONFIGURE_ARGS) build)
ifeq ($(WITH_GETTEXT),yes)
	@$(SED) -i.bak -e "/cd po &&/s|INSTALL_DATA=|DESTDIR= INSTALL_DATA=|" \
	    $(CLISP_BUILDDIR)/Makefile
endif

pre-install:
	@mkdir -p $(STAGEDIR)$(PREFIX)

post-install:
	@$(SED) -i.bak							\
	    -e 's| -I$(WRKSRC)/src/gllib||g'				\
	    -e 's| -I$(CLISP_BUILDDIR)/gllib||g'			\
	    -e 's| -I$(WRKSRC)/src||g'					\
	    -e 's|$(CLISP_BUILD_PREFIX)|$(PREFIX)|g'			\
	    $(STAGEDIR)$(CLISP_LIBDIR)/base/makevars
	@$(RM) $(STAGEDIR)$(CLISP_LIBDIR)/base/makevars.bak
	@perl -0777pi -e 'BEGIN { $$from=q{$(CLISP_BUILD_INCLUDEPATH)}; $$to=q{$(CLISP_RUNTIME_INCLUDEPATH)}; die "replacement too long\n" if length($$to) > length($$from); } s/\Q$$from\E/$$to . ("\0" x (length($$from) - length($$to)))/ge' \
	    $(STAGEDIR)$(CLISP_LIBDIR)/base/lisp.run
	@perl -0777pi -e 'BEGIN { $$from=q{$(CLISP_BUILD_LIBPATH)}; $$to=q{$(CLISP_RUNTIME_LIBPATH)}; die "replacement too long\n" if length($$to) > length($$from); } s/\Q$$from\E/$$to . ("\0" x (length($$from) - length($$to)))/ge' \
	    $(STAGEDIR)$(CLISP_LIBDIR)/base/lisp.run

post-stage:
ifeq ($(OPSYS),darwin)
	@codesign --force --sign - $(STAGEDIR)$(CLISP_LIBDIR)/base/lisp.run
endif

include $(PORTSDIR)/Mk/linux.port.post.mk
