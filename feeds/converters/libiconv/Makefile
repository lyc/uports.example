# Ports collection:	iconv
# Date created:		25/Jan/2008
# Whom:                 amanda@askey.com.tw

PORTNAME		= libiconv
DISTVERSION		= 1.18
CATEGORIES		= converters
MASTER_SITES		= $(MASTER_SITE_GNU)
MASTER_SITE_SUBDIR	= libiconv

MAINTAINER		= amanda.li@askey.com.tw
COMMENT			= A character set conversion library

LICENSE			= LGPL-2+ GPL-3+

USES			+= libtool

#USE_LDCONFIG		= yes
#USE_AUTOTOOLS 		= yes
GNU_CONFIGURE		= yes
CONFIGURE_ARGS		+= --enable-static				\
			   --docdir=$(PREFIX)/share/doc/$(PORTNAME)	\
			   --without-libiconv-prefix			\
			   --without-libintl-prefix			\
			   --disable-nls
ifeq ($(WITHOUT_EXTRA_ENCODINGS),)
CONFIGURE_ARGS		+= --enable-extra-encodings
endif

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.pre.mk

override_targets	+= post-patch pre-install post-install

post-patch:
	@$(SED) -e 's|%PREFIX%|$(PREFIX)|g ; s|%VERSION%|$(DISTVERSION)|g' \
		$(FILESDIR)/$(PORTNAME).pc.in > $(WRKDIR)/$(PORTNAME).pc

pre-install:
	@mkdir -p $(addprefix $(STAGEDIR)$(PREFIX)/,lib/pkgconfig)

post-install:
	@cp $(WRKDIR)/$(PORTNAME).pc $(STAGEDIR)$(PREFIX)/lib/pkgconfig

include $(PORTSDIR)/Mk/linux.port.post.mk
