# Ports collection      gforth
# Date created:         2025/03/03
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= gforth
DISTVERSION 		= 0.7.9
DISTVERSIONSUFFIX	= _20260610
CATEGORIES 		= devel
MASTER_SITES 		= https://www.complang.tuwien.ac.at/forth/gforth/Snapshots/$(DISTVERSIONFULL)/

USE_SCM			?= git
ifeq ($(USE_SCM),git)
SCM_PROTOCOL		= https://
MASTER_SITES		= git.savannah.gnu.org/
MASTER_SITE_SUBDIR	= git
SCM_DETACH		= $(DISTVERSIONFULL)
SCM_DEST		= $(DISTNAME)
endif

MAINTAINER 		= yowching.lee@gmail.com
COMMENT			= Fast and portable Forth system

LICENSE			= GPLv3+
LICENSE_FILE		= $(WRKSRC)/COPYING

# Gforth only build by GCC compiler!
ifeq ($(shell uname -s),Darwin)
GFORTH_DARWIN_GCC_PAIR	?= $(shell					\
	for n in 20 19 18 17 16 15 14 13 12 11 10 9 8 7; do		\
	    for p in /opt/homebrew /usr/local; do			\
	        if [ -x "$$p/bin/gcc-$$n" ] && [ -x "$$p/bin/g++-$$n" ]; then \
	            echo "$$p/bin/gcc-$$n $$p/bin/g++-$$n"; 		\
	            exit;						\
	        fi;							\
	    done;							\
	    if [ -x "/opt/local/bin/gcc-mp-$$n" ] &&			\
	       [ -x "/opt/local/bin/g++-mp-$$n" ]; then 		\
	        echo "/opt/local/bin/gcc-mp-$$n /opt/local/bin/g++-mp-$$n"; \
	        exit;							\
	    fi;								\
	done)
GFORTH_DARWIN_CC	= $(word 1,$(GFORTH_DARWIN_GCC_PAIR))
GFORTH_DARWIN_CXX	= $(word 2,$(GFORTH_DARWIN_GCC_PAIR))
ifeq ($(origin CC),default)
ifeq ($(GFORTH_DARWIN_CC),)
$(error gforth requires GNU GCC on Darwin; install gcc with Homebrew or MacPorts, or pass CC=/path/to/gcc CXX=/path/to/g++)
endif
CC			= $(GFORTH_DARWIN_CC)
endif
ifneq ($(filter default undefined,$(origin CPP)),)
ifneq ($(GFORTH_DARWIN_CC),)
CPP			= $(GFORTH_DARWIN_CC) -E
endif
endif
ifeq ($(origin CXX),default)
ifneq ($(GFORTH_DARWIN_CXX),)
CXX			= $(GFORTH_DARWIN_CXX)
endif
endif
endif

WITH_LIBFFCALL		?= no
WITH_DOCUMENT		?= no
WITH_X11		?= no

LIB_DEPENDS		= libffi.so:devel/libffi			\
			  libltdl.so:devel/libltdl
ifeq ($(WITH_LIBFFCALL),yes)
LIB_DEPENDS		+= libavcall.so:devel/libffcall
endif
BUILD_DEPENDS		+= autoconf>=2.69:devel/autoconf		\
			   automake>=1.16.1:devel/automake		\
			   gforth-bootstrap>=0.7.3:lang/gforth-0.7.3	\
			   libtool:devel/libtool

USES			= tar:xz libtool:build,keepla

USE_REINPLACE		= yes
GNU_CONFIGURE 		= yes
GFORTH_BOOT_PATH	= $(DESTDIR)$(PREFIX)/share/gforth/0.7.3:$(DESTDIR)$(PREFIX)/lib/gforth/0.7.3
GFORTH_BOOT_BIN		= $(DESTDIR)$(PREFIX)/bin/gforth-0.7.3
GFORTH_BOOT_LIBPATH	= $(DESTDIR)$(PREFIX)/lib
GFORTH_RUNTIME_LIBPATH	= $(PREFIX)/lib
GFORTH_TOOLS_PATH	= $(DESTDIR)$(PREFIX)/bin:$(PORTSDIR)/../../local/usr/bin
GFORTH_LIBCC_IMAGE_DEPS	= $$(buildccdir)/libgflibffi.la $$(buildccdir)/libgfpthread.la
ifeq ($(WITH_LIBFFCALL),yes)
GFORTH_LIBCC_IMAGE_DEPS	+= $$(buildccdir)/libgffflib.la
else
CONFIGURE_ENV		+= ac_cv_lib_avcall___builtin_avcall=no
endif

CONFIGURE_ENV		+= GFORTHHIST=$(WRKSRC) XDG_CACHE_HOME=$(WRKDIR)/.cache	\
			   GFORTHPATH=$(GFORTH_BOOT_PATH)		\
			   GFORTH=$(GFORTH_BOOT_BIN)			\
			   PATH=$(GFORTH_TOOLS_PATH):$$PATH
ifeq ($(WITH_DOCUMENT),no)
CONFIGURE_ENV		+= TEXI2DVI="echo texi2dvi disabled"		\
			   TEX="echo tex disabled"			\
			   TEXI2HTML="echo texi2html disabled"		\
			   texinfo_link=false
endif
ifeq ($(WITH_X11),no)
CONFIGURE_ENV		+= ac_cv_header_X11_X_h=no			\
			   ac_cv_header_X11_extensions_sync_h=no	\
			   ac_cv_header_X11_extensions_Xrandr_h=no	\
			   ac_cv_header_GL_glx_h=no			\
			   ac_cv_header_va_va_glx_h=no			\
			   ac_cv_header_va_va_x11_h=no
endif
MAKE_ENV		+= GFORTHHIST=$(WRKSRC) XDG_CACHE_HOME=$(WRKDIR)/.cache	\
			   CPATH=$(DESTDIR)$(PREFIX)/include:$$CPATH	\
			   LIBRARY_PATH=$(DESTDIR)$(PREFIX)/lib:$$LIBRARY_PATH \
			   LD_LIBRARY_PATH=$(DESTDIR)$(PREFIX)/lib:$$LD_LIBRARY_PATH \
			   PATH=$(GFORTH_TOOLS_PATH):$$PATH
LDFLAGS			+= -Wl,-rpath,$(GFORTH_RUNTIME_LIBPATH)

#
#
#

MASTERDIR 		= $(CURDIR)
PORTSDIR 		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk
override_targets	+= post-patch pre-configure post-install

post-patch:
	@$(REINPLACE_CMD) -e 's|@CC@|CFLAGS=-I$(DESTDIR)$(PREFIX)/include LDFLAGS=$(addprefix -L$(DESTDIR)$(PREFIX)/,$(libdirs))|g' $(WRKSRC)/envos.fs.in
	@$(REINPLACE_CMD) -e 's|env GFORTH_IGNLIB=true GFORTH_COMPILELIB=no HOSTPREFIX|env GFORTH_PREAMBLE="\#! @bindir@/gforth-@PACKAGE_VERSION@ --image-file" GFORTH_IGNLIB=true GFORTH_COMPILELIB=no HOSTPREFIX|' $(WRKSRC)/Makefile.in
	@$(REINPLACE_CMD) -e 's|^gforth\.fi: $$(buildccdir)/libgfpthread\.la @disasm2_la@|gforth.fi: $(GFORTH_LIBCC_IMAGE_DEPS) @disasm2_la@|' $(WRKSRC)/Makefile.in
ifeq ($(WITH_DOCUMENT),no)
	@$(REINPLACE_CMD) -e 's|^install-info:	doc|install-info:	info|' $(WRKSRC)/Makefile.in
	@$(REINPLACE_CMD) \
		-e '/doc\/gforth\.info\* doc\/vmgen\.info\*/s|.*|		@:|' \
		-e 's|^install-info:	info|install-info:|' \
		-e '/INSTALL-INFO/s|.*|		@:|' \
		-e '/MKDIR_P.*infodir/s|.*|		@:|' \
		-e '/INSTALL_INFO.*gforth\.info/s|.*|		@:|' \
		-e '/INSTALL_INFO.*vmgen\.info/s|.*|		@:|' \
		-e 's|^install-images:	install-md @INSTALLTXT@|install-images:	install-md|' \
		-e 's|^install-seq:	install-indep install-arch @INSTALLTXT@ @INSTALLINFO@|install-seq:	install-indep install-arch|' \
		-e 's|$$(man1dir) $$(infodir) $$(dataverdir)|$$(man1dir) $$(dataverdir)|' \
		-e 's|^doc:		info html ps pdf txt|doc:|' \
		-e 's|^doc:		info txt|doc:|' \
		$(WRKSRC)/Makefile.in
endif
	@$(RM) $(WRKSRC)/gforth.fi

pre-configure:
	@(cd $(WRKSRC) && env PATH=$(GFORTH_TOOLS_PATH):$$PATH ./autogen.sh)

post-install:
ifeq ($(WITH_DOCUMENT),no)
	@$(RM) $(STAGEDIR)$(PREFIX)/share/gforth/$(DISTVERSION)/doc/gforth.txt
endif
	@for f in $(STAGEDIR)/*/lib/gforth/$(DISTVERSION)/envos.fs; do \
		[ -e "$$f" ] || continue; \
		$(REINPLACE_CMD) -e 's|$(DESTDIR)$(PREFIX)|$(PREFIX)|g' "$$f"; \
	done
	@for f in $(STAGEDIR)/*/lib/gforth/$(DISTVERSION)/*/libcc-named/*.la; do \
		[ -e "$$f" ] || continue; \
		$(REINPLACE_CMD) -e 's|$(DESTDIR)$(PREFIX)|$(PREFIX)|g' "$$f"; \
	done
	@$(RM) $(STAGEDIR)/*/lib/gforth/$(DISTVERSION)/*/libcc-named/*.la.bak

include $(PORTSDIR)/Mk/linux.port.post.mk
