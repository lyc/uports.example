#
#
#

all: debug

topdir			:= $(shell pwd)
sitedir			:= $(topdir)/make
portdir			:= $(sitedir)/uports

#
# To be able to use GROUP capability (provided by tools.mk),
#
#   1. if extract uports packages existed, assign its location to FEEDS.
#   2. at least one group must be defeind (default "host" if not defined),
#      and its own DESTDIR and PREFIX must be provided.
#   3. then, include tools.mk to activate GROUP capability.
#

FEEDS			?= $(topdir)/feeds
DESTDIR			?= $(topdir)/local
PREFIX			?= /usr/local

# include ports package at here...
include $(portdir)/Tools/tools.mk

#
# add your own setting...
#

INCLUDES		= -I$(PREFIX)/include -I.
CPPFLAGS		+=
CFLAGS			=
CFLAGS_D		=
LDFLAGS			= -lpthread -lm
LDFLAGS_D		=

EXTRA_EXCLUDE_TARGETS	+= $(depends_exclude_targets) env i386-jos-elf

# add your all modules at here, please be noted
#
#  - It is not allow to have multiple files or folders with same name
#    in whole source tree
#  - application folder must be declared after library folder
#    in your module list
#

modules			+= libs/libutils
ifneq (,$(filter test,$(MAKECMDGOALS)))
modules			+= tests/01-utils
modules			+= tests/02-tree
modules			+= tests/03-values
modules			+= tests/04-stack
modules			+= tests/10-thread
modules			+= tests/20-json
modules			+= tests/21-expat
modules			+= tests/22-ezxml
else
modules			+= src
endif

#
# get all together...
#

include $(sitedir)/nonrecursive.mk

#
# define your own targets...
#

env: $(addsuffix .install,pkg-config cmake nijna meson)

.PHONY: test
test: debug

# Historical JOS producers run on the host, regardless of consumer selection.
EXPORT			?= no
EXPORT_DIR		?= $(topdir)/dist
JOS_SOURCE		?=
PATCHELF		?= $(host_DESTDIR)$(host_PREFIX)/bin/patchelf
toolchain-jos-targets	= $(addsuffix @mit2018.install,			\
			  toolchain:binutils				\
			  toolchain:gcc					\
			  toolchain:gdb					\
			  toolchain:qemu-mit)
toolchain-jos-export	= python3 -B "$(sitedir)/toolchain/export.py"	\
			  --profile i386-jos-elf			\
			  --project "$(topdir)"				\
			  --destdir "$(DESTDIR)"			\
			  --output "$(EXPORT_DIR)"			\
			  $(if $(strip $(JOS_SOURCE)),			\
			    --validation-source "$(JOS_SOURCE)")	\
			  --patchelf "$(PATCHELF)"

.PHONY: i386-jos-elf
i386-jos-elf:
	@case "$(EXPORT)" in yes|no) ;; *)				\
	    echo 'EXPORT must be yes or no' >&2; exit 1;; esac
	@if [ "$(EXPORT)" = yes ]; then $(toolchain-jos-export) --check; fi
	+$(MAKE) -j1 CROSS_COMPILE=					\
	  $(if $(filter yes,$(EXPORT)),host:patchelf.install)		\
	  $(toolchain-jos-targets)
	@if [ "$(EXPORT)" = yes ]; then $(toolchain-jos-export); fi

clean:
	@find . -type f -name \*~ -o -name .DS_Store | xargs rm -fr
distclean: ports.distclean .WAIT clean
	@rm -fr depend $(DESTDIR)
	@rm -fr $(addprefix $(portdir)/,distfiles packages scm)
