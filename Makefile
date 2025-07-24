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
PREFIX			?= /usr

PORTS_GROUP_DEFAULT	= textproc

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

EXTRA_EXCLUDE_TARGETS	+= $(depends_exclude_targets) env

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

env: $(addsuffix .install,pkg-config cmake)
test: debug
clean:
	@find . -type f -name \*~ -o -name .DS_Store | xargs rm -fr
distclean: ports.distclean clean
	@rm -fr depend $(DESTDIR)
	@rm -fr $(addprefix $(portdir)/,distfiles packages)
