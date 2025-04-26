#
# Special groups ...
#

default_ignore_lists	+= archivers/zlib				\
			   security/openssl

ignore_lists		+=

textproc_lists		= textproc/ezxml				\
			  textproc/expat2				\
			  textproc/json-c				\
			  textproc/libcsv				\
			  devel/pcre2					\
			  net/libyang2

PORTS_textproc_ENVS	+= $(PORTS_host_ENVS)				\
			   $(strip					\
			     PORTSDIR=$(portdir)			\
			     PREFIX=$(PREFIX)				\
			     DESTDIR=$(DESTDIR)/$(textproc)		\
			     $(if $(USE_ALTERNATIVE),			\
			       USE_ALTERNATIVE=$(USE_ALTERNATIVE),	\
			       USE_ALTERNATIVE=yes)			\
			     $(if $(ALTERNATIVE_WRKDIR),		\
			       ALTERNATIVE_WRKDIR=$(ALTERNATIVE_WRKDIR),\
			       ALTERNATIVE_WRKDIR=$(DESTDIR)/$(textproc)/src))

special_groups_all	+= textproc
