#
# Special groups ...
#

default_ignore_lists	+=

ignore_lists		+=

textproc_lists		= textproc/ezxml				\
			  textproc/expat2				\
			  textproc/mxml					\
			  textproc/json-c				\
			  textproc/libcsv				\
			  converters/libiconv				\
			  devel/libunistring				\
			  devel/pcre2					\
			  net/libyang2

PORTS_textproc_ENVS	+= $(PORTS_host_ENVS)				\
			   $(strip					\
			     PORTSDIR=$(portdir)			\
			     PREFIX=$(PREFIX)				\
			     DESTDIR=$(DESTDIR)/$(PORTS_GROUP_DEFAULT)	\
			     $(if $(USE_ALTERNATIVE),			\
			       USE_ALTERNATIVE=$(USE_ALTERNATIVE),	\
			       USE_ALTERNATIVE=yes)			\
			     $(if $(ALTERNATIVE_WRKDIR),		\
			       ALTERNATIVE_WRKDIR=$(ALTERNATIVE_WRKDIR),\
			       ALTERNATIVE_WRKDIR=$(DESTDIR)/$(PORTS_GROUP_DEFAULT)/src))

special_groups_all	+= textproc
