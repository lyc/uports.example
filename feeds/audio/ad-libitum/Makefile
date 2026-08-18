# Ports collection:	ad-libitum
# Date created:		2026/06/27
# Whom:			OpenAI Codex

PORTNAME		= ad-libitum
DISTVERSION		= 20260627
CATEGORIES		= audio

USE_SCM			= git
SCM_PROTOCOL		= https://
MASTER_SITES		= github.com/
MASTER_SITE_SUBDIR	= ul
SCM_DEST		= $(PORTNAME)-$(DISTVERSION)
SCM_DETACH		= b3cc6cf59deeac553bf070f26313353f1c0baec2
WRKSRC			= $(WRKDIR)/$(SCM_DEST)
SCM_SUBMODULES		= chez-soundio chez-sockets chez-portmidi \
			  org-html-themes

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= Scheme live coding environment built on Chez Scheme

LICENSE			= MIT

BUILD_DEPENDS		= scheme:lang/chezscheme
RUN_DEPENDS		= scheme:lang/chezscheme
LIB_DEPENDS		= libsoundio:audio/libsoundio

USE_PATCH		= V2

ADLIB_REMOTE_REPL	?= yes
ifeq ($(ADLIB_REMOTE_REPL),yes)
PATCHLIST_NAME		= series.remote
endif

NO_BUILD		=
ALL_TARGET		=
INSTALL_TARGET		=

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= do-build do-install

ifeq ($(OPSYS),darwin)
ADLIB_SHARED_FLAGS	= -dynamiclib -undefined dynamic_lookup
else
ADLIB_SHARED_FLAGS	= -shared -fPIC
endif

SCHEMEH			?= $(shell find $(DESTDIR)$(PREFIX)/lib -path '*/csv*/*/scheme.h' -print 2>/dev/null | head -1 | sed -e 's,/scheme.h$$,,')
ADLIBDIR		= $(PREFIX)/share/ad-libitum

do-build:
	@if [ -z "$(SCHEMEH)" ]; then					\
	    echo "Unable to find Chez Scheme scheme.h under $(DESTDIR)$(PREFIX)/lib"; \
	    echo "Install lang/chezscheme first, or pass SCHEMEH=/path/to/csv.../<machine>"; \
	    false;							\
	fi
	@$(MAKE) -C $(WRKSRC)/chez-soundio				\
	    CC="$(CC)"							\
	    CFLAGS="$(CFLAGS)"						\
	    LDFLAGS="$(LDFLAGS)"					\
	    SCHEMEH="$(SCHEMEH)"					\
	    PREFIX="$(DESTDIR)$(PREFIX)"				\
	    SHARED_FLAGS="$(ADLIB_SHARED_FLAGS)"
	@$(MAKE) -C $(WRKSRC)/chez-sockets				\
	    CC="$(CC)"							\
	    CFLAGS="$(CFLAGS)"						\
	    LDFLAGS="$(LDFLAGS)"					\
	    SCHEMEH="$(SCHEMEH)"					\
	    SHARED_FLAGS="$(ADLIB_SHARED_FLAGS)"
ifeq ($(OPSYS),darwin)
	@cd $(WRKSRC);							\
	for f in *.so; do						\
	    [ -f "$$f" ] || continue;					\
	    install_name_tool -delete_rpath $(DESTDIR)$(PREFIX)/lib "$$f" \
	        2>/dev/null || true;					\
	    install_name_tool -add_rpath $(PREFIX)/lib "$$f"		\
	        2>/dev/null || true;					\
	done
endif

do-install:
	@mkdir -p $(STAGEDIR)$(ADLIBDIR) $(STAGEDIR)$(PREFIX)/bin
	@cd $(WRKSRC);							\
	for f in *.ss *.so; do						\
	    [ -e "$$f" ] && cp -p "$$f" $(STAGEDIR)$(ADLIBDIR);		\
	done;								\
	for d in ad-libitum chez-soundio chez-sockets chez-portmidi srfi violet; do \
	    [ -d "$$d" ] && cp -R "$$d" $(STAGEDIR)$(ADLIBDIR);		\
	done
	@find $(STAGEDIR)$(ADLIBDIR)					\
	    \( -name .git -o -name .gitignore \) -exec rm -rf {} +
	@rm -f							\
	    $(STAGEDIR)$(ADLIBDIR)/chez-soundio/bridge.c		\
	    $(STAGEDIR)$(ADLIBDIR)/chez-soundio/sine.c			\
	    $(STAGEDIR)$(ADLIBDIR)/chez-soundio/tangle.sh		\
	    $(STAGEDIR)$(ADLIBDIR)/chez-soundio/Makefile		\
	    $(STAGEDIR)$(ADLIBDIR)/chez-sockets/Makefile		\
	    $(STAGEDIR)$(ADLIBDIR)/chez-sockets/socket-ffi-values.c	\
	    $(STAGEDIR)$(ADLIBDIR)/chez-sockets/sockets-stub.c
ifeq ($(ADLIB_REMOTE_REPL),yes)
	@printf '%s\n'							\
	    '(load "ad-libitum.ss")'					\
	    '(repl:start-repl-server)'					\
	    '(let loop () (sound:flush-events!) (repl:process-pending!) (sleep (make-time '\''time-duration 50000000 0)) (loop))' \
	    > $(STAGEDIR)$(ADLIBDIR)/ad-libitum-remote-repl.ss
endif
	@printf '%s\n'							\
	    '#!/bin/sh'							\
	    'self=$$(CDPATH= cd -- "$$(dirname -- "$$0")" && pwd)'	\
	    'prefix=$$(dirname -- "$$self")'				\
	    'cd "$$prefix/share/ad-libitum" || exit 1'			\
	    'scheme_bin=$$self/scheme'					\
	    '[ -x "$$scheme_bin" ] || scheme_bin=scheme'		\
	    'if [ "$${1:-}" = "--remote-repl" ]; then'			\
	    '    shift'							\
	    '    exec "$$scheme_bin" ad-libitum-remote-repl.ss "$$@"'	\
	    'fi'							\
	    'if [ "$$#" -eq 0 ]; then'					\
	    '    exec "$$scheme_bin" ad-libitum.ss'			\
	    'fi'							\
	    'exec "$$scheme_bin" "$$@"'					\
	    > $(STAGEDIR)$(PREFIX)/bin/ad-libitum
	@chmod 755 $(STAGEDIR)$(PREFIX)/bin/ad-libitum
ifeq ($(ADLIB_REMOTE_REPL),yes)
	@cp -p $(MASTERDIR)/files/ad-libitum-repl			\
	    $(STAGEDIR)$(PREFIX)/bin/ad-libitum-repl
	@chmod 755 $(STAGEDIR)$(PREFIX)/bin/ad-libitum-repl
endif

include $(PORTSDIR)/Mk/linux.port.post.mk
