# Ports collection:     gmp
# Date created:         3/March/2013
# Whom:                 Max Lee

PORTNAME		= gmp
DISTVERSION		= 6.3.0
PORTREVISION		= 0
CATEGORIES		= devel
MASTER_SITES		= https://gmplib.org/download/gmp/

MAINTAINER		= yowching.lee@gmail.com
COMMENT			= The GNU Multiple Precision Arithmetic Library

LICENSE			= LGPLv3 GPLv2

USES			+= tar:xz libtool

GNU_CONFIGURE		= yes
CONFIGURE_SCRIPT	= configure
CONFIGURE_ENV		+= CPPFLAGS=-fexceptions

# GMP 6.3.0 x86_64 assembly is not package-safe for shared Darwin builds in
# this tree: the release libtool path links but mpn tests segfault, and the
# autoreconf/PIC path still fails the dylib link on RIP-relative addressing.
# Set GMP_ASM=safe or GMP_ASM=native only for local experiments.
ifeq ($(shell uname -s),Darwin)
GMP_DARWIN_MAJOR	:= $(shell uname -r | sed 's/\..*//')
ifeq ($(shell uname -m),arm64)
GMP_ASM			?= safe
GMP_DARWIN_CPU		?= aarch64
else
GMP_ASM			?= off
GMP_DARWIN_CPU		?= x86_64
endif
else
GMP_ASM			?= safe
endif
ifeq ($(shell uname -s),Darwin)
ifneq ($(GMP_ASM),off)
USES			+= autoreconf
GMP_LOCALBASE_TOOLS	= $(abspath $(CURDIR)/../../..)/local$(LOCALBASE)/bin
AUTORECONF		?= env PATH=$(GMP_LOCALBASE_TOOLS):$$PATH autoreconf
endif
ifeq ($(GMP_ASM),off)
CONFIGURE_ARGS		+= --disable-assembly
else ifeq ($(GMP_ASM),safe)
CONFIGURE_ARGS		+= --build=$(GMP_DARWIN_CPU)-apple-darwin$(GMP_DARWIN_MAJOR) \
			   ABI=64
endif
endif
CONFIGURE_ARGS		+= --enable-cxx					\
			   --with-pic					\
			   --with-gnu-ld

WITH_DOCUMENT		?= no

ifneq ($(WITH_DOCUMENT),yes)
override_targets	+= post-configure

post-configure:
	@$(SED) -i.bak -e 's|^SUBDIRS = tests mpn mpz mpq mpf printf scanf rand cxx demos tune doc|SUBDIRS = tests mpn mpz mpq mpf printf scanf rand cxx demos tune|' $(CONFIGURE_WRKSRC)/Makefile
endif

ALL_TARGET		= all check

# Libraries use build/host configuration, including CROSS_COMPILE when set.
# CONFIGURE_TARGET selects compiler output, not this library architecture.
# Keep PREFIX and work layout under project control.
ifneq ($(CONFIGURE_TARGET),)
override CONFIGURE_TARGET	:=
endif

#
#
#

MASTERDIR		= $(CURDIR)
PORTSDIR		?= $(MASTERDIR)/../..
include $(PORTSDIR)/Mk/linux.port.mk
