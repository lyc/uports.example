# Prts collection:	ezxml
# Date created:		05/May/2023
# Whom:                 yowching.lee@gmail.com.tw

PORTNAME		= ezxml
PORTVERSION		= 0.8.6
CATEGORIES		= textproc
MASTER_SITES		= $(MASTER_SITE_SOURCEFORGE)
MASTER_SITE_SUBDIR	= $(PORTNAME)/ezXML/ezXML%20$(PORTVERSION)

MAINTAINER		= ports@FreeBSD.org
COMMENT			= Easy to use C library for parsing XML documents

LICENSE			= MIT

USES			= cpe
CPE_VENDOR		= $(PORTNAME)_project

MAKEFILE		= GNUmakefile
WRKSRC			= $(WRKDIR)/$(PORTNAME)

LIBFILES		= libezxml.a
HEADERFILES		= ezxml.h

#
#
#

MASTERDIR 		= $(CURDIR)
PORTSDIR 		?= $(MASTERDIR)/../..

override_targets 	+= post-patch do-install

post-patch:
	@$(SED) -e 's|%PREFIX%|$(PREFIX)|g ; s|%VERSION%|$(PORTVERSION)|g' \
		$(FILESDIR)/$(PORTNAME).pc.in > $(WRKDIR)/$(PORTNAME).pc

do-install:
	@for d in include lib share/$(PORTNAME) lib/pkgconfig; do	\
	    mkdir -p $(STAGEDIR)$(PREFIX)/$${d};			\
	done
	@cp $(WRKSRC)/$(LIBFILES) $(STAGEDIR)$(PREFIX)/lib
	@cp $(WRKSRC)/$(HEADERFILES) $(STAGEDIR)$(PREFIX)/include
	@cp $(WRKSRC)/ezxml.txt $(STAGEDIR)$(PREFIX)/share/$(PORTNAME)
	@cp $(WRKDIR)/$(PORTNAME).pc $(STAGEDIR)$(PREFIX)/lib/pkgconfig

include $(PORTSDIR)/Mk/linux.port.mk
