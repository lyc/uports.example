# Ports collection:	bzip2
# Date created:		2026/06/06
# Whom:			Max Lee <yowching.lee@gmail.com>

PORTNAME		= bzip2
DISTVERSION		= 1.0.8
CATEGORIES		= archivers
MASTER_SITES		= https://sourceware.org/pub/bzip2/

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= Block-sorting file compressor

LICENSE			= BSD4CLAUSE
LICENSE_FILE		= $(WRKSRC)/LICENSE

USES			=

MAKE_ARGS		+= CC="$(CC)" AR="$(AR)" RANLIB="$(RANLIB)"	\
			   CFLAGS="$(CFLAGS) -fPIC"

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= do-build do-install

do-build:
	@(cd $(BUILD_WRKSRC) && $(SETENV) $(MAKE_ENV) $(MAKE) $(MAKE_ARGS))
	@(cd $(BUILD_WRKSRC) && $(SETENV) $(MAKE_ENV) $(MAKE) -f Makefile-libbz2_so CC="$(CC)" CFLAGS="$(CFLAGS) -fPIC")

do-install:
	@$(MKDIR) $(STAGEDIR)$(PREFIX)/bin
	@$(MKDIR) $(STAGEDIR)$(PREFIX)/include
	@$(MKDIR) $(STAGEDIR)$(PREFIX)/lib
	@$(MKDIR) $(STAGEDIR)$(PREFIX)/share/man/man1
	@/usr/bin/install -c -m 755 $(WRKSRC)/bzip2 $(STAGEDIR)$(PREFIX)/bin/bzip2
	@$(LN) -sf bzip2 $(STAGEDIR)$(PREFIX)/bin/bunzip2
	@$(LN) -sf bzip2 $(STAGEDIR)$(PREFIX)/bin/bzcat
	@/usr/bin/install -c -m 755 $(WRKSRC)/bzdiff $(STAGEDIR)$(PREFIX)/bin/bzdiff
	@$(LN) -sf bzdiff $(STAGEDIR)$(PREFIX)/bin/bzcmp
	@/usr/bin/install -c -m 755 $(WRKSRC)/bzgrep $(STAGEDIR)$(PREFIX)/bin/bzgrep
	@$(LN) -sf bzgrep $(STAGEDIR)$(PREFIX)/bin/bzegrep
	@$(LN) -sf bzgrep $(STAGEDIR)$(PREFIX)/bin/bzfgrep
	@/usr/bin/install -c -m 755 $(WRKSRC)/bzmore $(STAGEDIR)$(PREFIX)/bin/bzmore
	@$(LN) -sf bzmore $(STAGEDIR)$(PREFIX)/bin/bzless
	@/usr/bin/install -c -m 644 $(WRKSRC)/bzlib.h $(STAGEDIR)$(PREFIX)/include/bzlib.h
	@/usr/bin/install -c -m 644 $(WRKSRC)/libbz2.a $(STAGEDIR)$(PREFIX)/lib/libbz2.a
	@/usr/bin/install -c -m 755 $(WRKSRC)/libbz2.so.$(DISTVERSION) $(STAGEDIR)$(PREFIX)/lib/libbz2.so.$(DISTVERSION)
	@$(LN) -sf libbz2.so.$(DISTVERSION) $(STAGEDIR)$(PREFIX)/lib/libbz2.so.1.0
	@$(LN) -sf libbz2.so.$(DISTVERSION) $(STAGEDIR)$(PREFIX)/lib/libbz2.so
	@/usr/bin/install -c -m 644 $(WRKSRC)/bzip2.1 $(STAGEDIR)$(PREFIX)/share/man/man1/bzip2.1
	@$(LN) -sf bzip2.1 $(STAGEDIR)$(PREFIX)/share/man/man1/bunzip2.1
	@$(LN) -sf bzip2.1 $(STAGEDIR)$(PREFIX)/share/man/man1/bzcat.1

include $(PORTSDIR)/Mk/linux.port.post.mk
