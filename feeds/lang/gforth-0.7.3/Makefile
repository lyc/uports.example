# Ports collection      gforth
# Date created:         2025/03/03
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= gforth
DISTVERSION 		= 0.7.3
PKGNAMESUFFIX		= -bootstrap
CATEGORIES 		= devel
MASTER_SITES 		= $(MASTER_SITE_GNU)
MASTER_SITE_SUBDIR	= $(PORTNAME)

MAINTAINER 		= yowching.lee@gmail.com
COMMENT			= Fast and portable Forth system

LICENSE			= GPLv3+
LICENSE_FILE		= $(WRKSRC)/COPYING

LIB_DEPENDS		= libavcall.so:devel/libffcall			\
			  libffi.so:devel/libffi			\
			  libltdl.so:devel/libltdl

USES			= libtool:build

USE_REINPLACE		= yes
GNU_CONFIGURE 		= yes
CONFIGURE_ENV		+= GFORTHHIST=$(WRKSRC)
MAKE_ENV		+= GFORTHHIST=$(WRKSRC)

# NOTE: don't use libtool (which provided by devel/libtool)
export ac_cv_prog_GNU_LIBTOOL	:=

#
#
#

MASTERDIR 		= $(CURDIR)
PORTSDIR 		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk
override_targets	+= post-patch pre-configure post-install

post-patch:
	@cp $(PORTSDIR)/Tools/config.guess $(WRKSRC)/config.guess
	@cp $(PORTSDIR)/Tools/config.sub $(WRKSRC)/config.sub
	@$(REINPLACE_CMD) -e 's|@CC@|CFLAGS=-I$(DESTDIR)$(PREFIX)/include LDFLAGS=$(addprefix -L$(DESTDIR)$(PREFIX)/,$(libdirs))|g' $(WRKSRC)/envos.fs.in
# don't search PATH for a host gforth, forces the bootstrap path where the freshly built local engine uses the bundled kernl*.fi images through ./preforth.
	@$(REINPLACE_CMD) -e 's|GFORTH="`cd / && which gforth 2>/dev/null`"|GFORTH=""|g' $(WRKSRC)/configure
	@$(REINPLACE_CMD) -e 's|GFORTH="`cd / && which gforth 2>/dev/null`"|GFORTH=""|g' $(WRKSRC)/configure.in
	@$(REINPLACE_CMD) \
		-e '/doc\/gforth\.info\* .*doc\/vmgen\.info\*/s|.*|		@:|' \
		-e '/INSTALL_INFO.*gforth\.info/s|.*|		@:|' \
		-e '/INSTALL_INFO.*vmgen\.info/s|.*|		@:|' \
		-e 's|$$(man1dir) $$(infodir) $$(libdir)|$$(man1dir) $$(libdir)|' \
		$(WRKSRC)/Makefile.in

pre-configure:
	@cp $(PORTSDIR)/Tools/config.guess $(WRKSRC)/config.guess
	@cp $(PORTSDIR)/Tools/config.sub $(WRKSRC)/config.sub

post-install:
	@$(RM) $(STAGEDIR)$(PREFIX)/bin/gforth
	@$(RM) $(STAGEDIR)$(PREFIX)/bin/gforth-fast
	@$(RM) $(STAGEDIR)$(PREFIX)/bin/gforth-itc
	@$(RM) $(STAGEDIR)$(PREFIX)/bin/gforthmi
	@$(RM) $(STAGEDIR)$(PREFIX)/bin/vmgen
	@$(RM) $(STAGEDIR)$(PREFIX)/share/man/man1/gforth.1
	@$(RM) $(STAGEDIR)$(PREFIX)/share/info/gforth.info*
	@$(RM) $(STAGEDIR)$(PREFIX)/share/info/vmgen.info
	@$(RM) $(STAGEDIR)$(PREFIX)/share/info/dir
	@$(RM) $(STAGEDIR)$(PREFIX)/share/gforth/site-forth/siteinit.fs

include $(PORTSDIR)/Mk/linux.port.post.mk
