# Ports collection      clisp
# Date created:         2025/10/28
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= clisp
DISTVERSION 		= 2.49.95
CATEGORIES 		= lang
MASTER_SITES		= https://anduin.linuxfromscratch.org/BLFS/clisp/

USE_SCM			?= git
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
WITH_DOCUMENT		?= yes

ifeq ($(shell uname -s),Darwin)
override WITH_GETTEXT	= no
endif

ifeq ($(WITH_DOCUMENT),no)
PLIST_NAME		= pkg-plist-nodoc
endif

LIB_DEPENDS		= libavcall.so:devel/libffcall			\
			  libcallback.so:devel/libffcall
ifeq ($(WITH_READLINE),yes)
LIB_DEPENDS		+= libreadline.so:devel/readline			\
			   libncurses.so.6:devel/ncurses
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
CONFIGURE_ARGS		+= --without-gettext				\
			   --disable-nls
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
ifeq ($(WITH_DOCUMENT),no)
	@$(SED) -i.bak							\
	    -e 's|^\(all : .*\) manual \(.*\)|\1 \2|'			\
	    -e 's|^install : install-bin install-man install-doc|install : install-bin install-man|' \
	    -e 's|^install-man : .* force|install-man : clisp.1 clisp-link.1 force|' \
	    $(CLISP_BUILDDIR)/Makefile
endif

pre-install:
	@mkdir -p $(STAGEDIR)$(PREFIX)

post-install:
ifeq ($(WITH_DOCUMENT),no)
	@$(RM) -r $(STAGEDIR)$(PREFIX)/share/doc/$(PORTNAME)
	@rmdir $(STAGEDIR)$(PREFIX)/share/doc 2>/dev/null || :
else
	@$(RM)							\
	    $(STAGEDIR)$(PREFIX)/share/doc/$(PORTNAME)/clisp.html	\
	    $(STAGEDIR)$(PREFIX)/share/doc/$(PORTNAME)/clisp.pdf	\
	    $(STAGEDIR)$(PREFIX)/share/doc/$(PORTNAME)/clisp.ps		\
	    $(STAGEDIR)$(PREFIX)/share/doc/$(PORTNAME)/clisp-link.html	\
	    $(STAGEDIR)$(PREFIX)/share/doc/$(PORTNAME)/clisp-link.pdf	\
	    $(STAGEDIR)$(PREFIX)/share/doc/$(PORTNAME)/clisp-link.ps
endif
	@$(SED) -i.bak							\
	    -e 's| -I$(WRKSRC)/src/gllib||g'				\
	    -e 's| -I$(CLISP_BUILDDIR)/gllib||g'			\
	    -e 's| -I$(WRKSRC)/src||g'					\
	    -e 's|$(CLISP_BUILD_PREFIX)|$(PREFIX)|g'			\
	    $(STAGEDIR)$(CLISP_LIBDIR)/base/makevars
	@$(RM) $(STAGEDIR)$(CLISP_LIBDIR)/base/makevars.bak
	@bash -ec '\
	    rewrite_path() {						\
	        file=$$1; from=$$2; to=$$3;				\
	        from_len=$${#from}; to_len=$${#to};			\
	        if [ $$to_len -gt $$from_len ]; then			\
	            echo "replacement too long: $$to > $$from" >&2;	\
	            exit 1;						\
	        fi;							\
	        pad=$$((from_len - to_len));				\
	        grep -aboF "$$from" "$$file" | while IFS=: read -r off _; do \
	            { printf "%s" "$$to"; head -c "$$pad" /dev/zero; } \
	                | dd of="$$file" bs=1 seek="$$off" conv=notrunc 2>/dev/null; \
	        done;							\
	    };								\
	    rewrite_path "$(STAGEDIR)$(CLISP_LIBDIR)/base/lisp.run"	\
	        "$(CLISP_BUILD_INCLUDEPATH)" "$(CLISP_RUNTIME_INCLUDEPATH)"; \
	    rewrite_path "$(STAGEDIR)$(CLISP_LIBDIR)/base/lisp.run"	\
	        "$(CLISP_BUILD_LIBPATH)" "$(CLISP_RUNTIME_LIBPATH)"'

post-stage:
ifeq ($(OPSYS),darwin)
	@codesign --force --sign - $(STAGEDIR)$(CLISP_LIBDIR)/base/lisp.run
endif

include $(PORTSDIR)/Mk/linux.port.post.mk
