# Ports collection      lua
# Date created:         2013/11/18
# Whom:                 Max Lee <max_lee@pegatroncorp.com>

PORTNAME		= lua
DISTVERSION 		= 5.4.7
CATEGORIES 		= lang
MASTER_SITES 		= http://www.lua.org/ftp/
LICENSE			= MIT

MAINTAINER 		= max_lee@pegatroncorp.com
COMMENT			= Small, compilable scripting language providing easy access to C code

LICENSE			= MIT

USE_REINPLACE		= yes

LIB_DEPENDS		= libreadline.so.6:devel/readline

#
#
#

MASTERDIR 		= $(CURDIR)
PORTSDIR 		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= post-patch do-configure pre-install post-install
ifeq ($(OPSYS),linux)
  plat			:= linux
else
  ifeq ($(OPSYS),darwin)
    plat		:= macosx
  endif
endif
ALL_TARGET		= $(plat)
MAKE_ENV		+= PLAT=$(plat)					\
			   INSTALL_TOP=$(PREFIX)

post-patch:
	@for f in Makefile src/luaconf.h; do 				\
	    $(REINPLACE_CMD) -e "s|%PREFIX%|$(PREFIX)|" $(WRKSRC)/$$f;	\
	done
	@$(REINPLACE_CMD) 						\
	    -e "s|\(^MYCFLAGS=\)\(.*$$\)|\1 -I$(DESTDIR)$(PREFIX)/include|"\
	    -e "s|\(^MYLDFLAGS=\)\(.*$$\)|\1 -L$(DESTDIR)$(PREFIX)/lib|"\
	    $(WRKSRC)/src/Makefile
	@$(SED) -e 's|%PREFIX%|$(PREFIX)|g ; s|%VERSION%|$(PORTVERSION)|g' \
		$(FILESDIR)/$(PORTNAME).pc.in > $(WRKDIR)/$(PORTNAME).pc

do-configure:

pre-install:
	@mkdir -p $(addprefix $(STAGEDIR)$(PREFIX)/,lib/pkgconfig)

post-install:
	@cp $(WRKDIR)/$(PORTNAME).pc $(STAGEDIR)$(PREFIX)/lib/pkgconfig

include $(PORTSDIR)/Mk/linux.port.post.mk
