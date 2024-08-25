#
#
#

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

# include ports package at here...
include $(portdir)/Tools/tools.mk

#
# define your own targets...
#

env: $(addsuffix .install,pkg-config cmake) 

clean:
	@find . -type f -name \*~ -o -name .DS_Store | xargs rm -fr

distclean: ports.distclean clean
	@rm -fr depend $(DESTDIR)
	@rm -fr $(addprefix $(portdir)/,distfiles packages)
