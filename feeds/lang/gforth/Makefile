# Ports collection      gforth
# Date created:         2025/03/03
# Whom:                 Max Lee <yowching.lee@gmail.com>

PORTNAME		= gforth
DISTVERSION 		= 0.7.3
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
override_targets	+= post-patch

post-patch:
	@$(REINPLACE_CMD) -e 's|@CC@|CFLAGS=-I$(DESTDIR)$(PREFIX)/include LDFLAGS=$(addprefix -L$(DESTDIR)$(PREFIX)/,$(libdirs))|g' $(WRKSRC)/envos.fs.in

include $(PORTSDIR)/Mk/linux.port.post.mk
