#
#  ...
#

# $(subdirectory)
subdirectory		= $(patsubst %/$1,%,				\
			    $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))

pwd			:= $(call subdirectory,nonrecursive.mk)

DEBUG.MK		:= $(pwd)/linux.debug.mk
CONFIG.GUESS		:= $(pwd)/config.guess



#
# utilities ...
#

# cpu-vendor-os
# cpu-vendor-kernel-system

tripulet.cpu		= $(word 1,$(subst -, ,$1))
tripulet.os		= $(strip					\
			    $(if $(findstring linux,$1),		\
			       $(word 2, $(subst -linux, linux,$1)),	\
			       $(if $(findstring darwin,$1),		\
				 osx,					\
			         $(error Oops, can't identiry system)))) #'
tripulet.kernel		?= $(word 1,$(subst -, ,$(call tripulet.os,$1)))
tripulet.system		?= $(word 2,$(subst -, ,$(call tripulet.os,$1)))

#
nth			= $(word $1,$(subst ., ,$2))
rmpath			= $(subst $1/,,$2)
addpath			= $(addprefix $1/,$2)

# $(call ver, lib-real-name-without-prefix-path)
ver			= $(filter-out lib% so dylib,$(subst ., ,$1))
lib_name		= $(filter lib%,$(subst ., ,$1))
lib_version		= $(shell echo $(call ver,$1) | sed -e 's/ /\./'g)
lib_major		= $(firstword $(call ver,$1))
lib_minor		= $(shell echo $(wordlist 2,3,$(call ver,$1)) | sed -e 's/ /\./'g)

# $(call filter-srcs, source-file-list)
filter-srcs		= $(filter-out %.$(LIB_EXT) %.$(OBJ_EXT),$1)

# $(call source-to-object, source-file-list)
srcs-to-objs		= $(subst .c,.$(OBJ_EXT),$(filter %.c,$1))	\
			  $(subst .y,.$(OBJ_EXT),$(filter %.y,$1))	\
			  $(subst .l,.$(OBJ_EXT),$(filter %.l,$1))

# $(call srcs-to-paths, source-file-list)
srcs-to-paths		= $(sort $(dir $(call filter-srcs,$1)))

# $(call add-incs, incs, module-dir, source-file-list)
add-incs		= $1 $(addprefix -I,				\
			       $(filter-out $(patsubst -I%,%,$1),	\
			         $(patsubst %/.,%,			\
			           $(call addpath,$2,			\
			             $(patsubst %/,%,			\
			               $(call srcs-to-paths,$3))))))

# $(call make-vpath, module-dir, source-file-list)
make-vpath		= $(shell echo 					\
			    $(addprefix $1/,$(call srcs-to-paths,$2))	\
			    | sed -e 's/ /:/g' -e 's/[^\.]\.\///g')

# Convert "$(OBJ_DIR)/libABC.$(LIB_EXT)" to "-L$(OBJ_DIR) -lABC"
# $(call xfer-libs, OBJ_DIR, prerequisite, LIB_EXT)
xfer-libs		= $(if $(filter %.$3,$2),			\
			    -L$1					\
			    $(addprefix -l,				\
			      $(patsubst $1/lib%.$3,%,			\
			        $(filter %.$3,$2))),)

##
# $(call s_rdeps, name)
define s_rdeps
  $(if $($1_deps), 							\
    $($1_deps) $(foreach t,$($1_deps),$(call s_rdeps,$t)),)
endef

# $(call m_rdeps, names ...)
define m_rdeps
  $(foreach t,$1,$(call s_rdeps,$t))
endef

# $(call all_rdeps, names ...)
define all_rdeps
  $(foreach t,$1,$t $(call s_rdeps,$t))
endef

# $(warning PKG_CONFIG=$(PKG_CONFIG))
# $(warning PKGCONFIG_CMD=$(PKGCONFIG_CMD))
#	echo requires_all=$(requires_all);				\
#	echo HOSTTOOLS=$(USE_HOSTTOOLS);				\
#	ext=$(call run-pkg-config,--cflags,$(requires_all));		\
#	ext=`if [ -e $(PKG_CONFIG) ] && [ ! -z "$(requires_all)" ]; then $(PKGCONFIG_CMD) --cflags $(requires_all); fi`;	\
#	echo ext=$$ext;							\

ifneq ($(USE_HOSTTOOLS),)
  run-pkg-config	= $(shell if [ ! -z "$2" ]; then		\
				    if [ -e $(PKG_CONFIG) ]; then	\
				      $(PKGCONFIG_CMD) $1 $2;		\
				    fi;					\
				  fi)
else
  run-pkg-config	=
endif

include $(DEBUG.MK)



#
# host.mk ...
#

SED			= sed
RM			= rm

ARCHIVE_DIR		:= $(DESTDIR)$(PREFIX)/lib
DEPEND_DIR		:= $(if $(builddir),$(builddir)/depend,depend)
OBJ_DIR			:= $(if $(builddir),$(builddir)/obj,obj)
OBJ_DEBUG_DIR		:= $(if $(builddir),$(builddir)/objd,objd)
OBJ_PROFILE_DIR		:= $(if $(builddir),$(builddir)/objp,objp)



#
# target.mk ...
# 
#   1. toolchain support
#

ifneq ($(CROSS_COMPILE),)
TRIPLET			:= $(CROSS_COMPILE)
TOOLS_PREFIX		= $(CROSS_COMPILE)-
endif

# FIXME: workaround for old Mac OS/X which uname will report wrong value
ifeq ($(shell uname -s),Darwin)
TRIPLET			:= $(shell $(CONFIG.GUESS))
endif

ifeq ($(TRIPLET),)
OPSYS			?= $(shell uname -s | tr '[:upper:]' '[:lower:]' |\
			    sed -e 's/darwin/osx/')
ARCH			?= $(shell uname -m)
else
ARCH			?= $(call tripulet.cpu,$(TRIPLET))
OPSYS			?= $(call tripulet.kernel,$(TRIPLET))
OPSYS_SUFX		?= $(call tripulet.system,$(TRIPLET))
endif

# $(warning OPSYS=$(OPSYS))
# $(warning OPSYS_SUFX=$(OPSYS_SUFX))
# $(warning ARCH=$(ARCH))

AS			= $(TOOLS_PREFIX)as
CC			= $(TOOLS_PREFIX)gcc
CXX			= $(TOOLS_PREFIX)g++
LD			= $(TOOLS_PREFIX)ld
STRIP			= $(TOOLS_PREFIX)strip
OBJCOPY			= $(TOOLS_PREFIX)objcopy
OBJDUMP			= $(TOOLS_PREFIX)objdump
NM			= $(TOOLS_PREFIX)nm
AR			= $(TOOLS_PREFIX)ar
RANLIB			= $(TOOLS_PREFIX)ranlib

# CPP = $(CC) -E
# COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
# COMPILE.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
# LINK.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)
# LINK.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)
#
# $(TARGET_ARCH)
# $(CPPFLAGS)
# $(CC)
# $(CFLAGS)
# $(CXX)
# $(CXXFLAGS)
# $(LDFLAGS)

INCLUDES		:= $(INCLUDES)

ifeq ($(OPSYS),linux)
  ifeq ($(ARCH),x86_64)
    CPPFLAGS		+= -D__LINUX__ -D__X86_64__
  else
    CPPFLAGS		+= -D__LINUX__
  endif
else
  CPPFLAGS		+= -D__OSX__
endif
CFLAGS			+= -W -Wall -Wextra
CFLAGS_D		+= -g -DDEBUG
LDFLAGS			+=
LDFLAGS_D		+=
EXTRA_CLEAN		:= $(EXTRA_CLEAN)

LIBRARY			?= shared

DEPEND_EXT		:= d
OBJ_EXT			:= o

ifeq ($(LIBRARY),shared)
  ifeq ($(OPSYS),linux)
    CFLAGS		+= -fPIC
    LDFLAGS		+= -shared
    LDFLAGS_D		+= -g $(LDFLAGS)
  else
    CFLAGS		+= -fno-common
    LDFLAGS		+= -dynamiclib
    LDFLAGS_D		+= $(LDFLAGS)
  endif
endif

ifeq ($(LIBRARY),shared)
  ifeq ($(OPSYS),linux)
    LIB_EXT		:= so

    shared_ext		= $(LIB_EXT).$1 # so.0.1.3
    soname		= $(call lib_name,$1).$(LIB_EXT).$(call lib_major,$1) # libXXX.so.0
    shared_opts		= -Wl,-soname,$(call soname,$1)
    shared_cmds		= /sbin/ldconfig -n .$(trash)
  else
    LIB_EXT		:= dylib

    shared_ext		= $1.$(LIB_EXT) # 0.1.2.dylib
    soname		= $(call lib_name,$1).$(call lib_major,$1).$(LIB_EXT) # libXXX.0.dylib
    shared_opts		= -install_name $(call soname,$1) -compatibility_version $(call lib_major,$1) -current_version $(call lib_version,$1)
    shared_cmds		= if [ $1 != $$(call soname,$1) ]; then ln -sf $1 $$(call soname,$1); fi
  endif
else
  ifeq ($(OPSYS),linux)
    LIB_EXT		:= a
  else
    $(error Oops, only support static library under Linux)
  endif
endif


incs_all		:=			# list of all include path
srcs_all		:=			# list of all source
libs_all		:=			# list of all library
plugins_all		:=			# list of all plugin
apps_all		:=			# list of all application
requires_all		:=



#
# rules.mk...
#

#####
# archive copy rules...

# NOTE:
# Extra archives can be added by following steps:
#  - Put archives in $(ARCHIVE_DIR) folder, default is $(DESTDIR)$(PREFIX)/lib
#  - Then add $(addprefix $(ARCHIVE_DIR)/,__your_archives_list__)
#    in module.mk srcs list
#
# For example:
#  - Most of switch chip vendors SDK was built with archive (*.a) output format
#  - Use Ports system to build SDK and install its archive output into
#    $(DESTDIR)$(PREFIX)/lib folder
#  - Create SDK_ARCHIVES makefile variable with SDK's archives list
#  - Add $(addprefix $(ARCHIVE_DIR)/,$(SDK_ARCHIVES)) in application module.mk
#    which need SDK's archives

quiet_cmd_install	?= INSTALL $(patsubst $(ARCHIVE_DIR)/%,%,$<)
      cmd_install	?= set -e; mkdir -p $(OBJ_DIR);	cp $< $@

quiet_cmd_install_d	?= INSTALL $(patsubst $(ARCHIVE_DIR)/%,%,$<)
      cmd_install_d	?= set -e; mkdir -p $(OBJ_DEBUG_DIR); cp $< $@

$(OBJ_DIR)/%.o: $(ARCHIVE_DIR)/%.o
	$(call cmd,install)

$(OBJ_DEBUG_DIR)/%.o: $(ARCHIVE_DIR)/%.o
	$(call cmd,install_d)

$(OBJ_DIR)/%.a: $(ARCHIVE_DIR)/%.a
	$(call cmd,install)

$(OBJ_DEBUG_DIR)/%.a: $(ARCHIVE_DIR)/%.a
	$(call cmd,install_d)

#####
# depend rules...

# NOTE:
#  Below make function call can't work properly under Linux, so we need to
#  use shell script directly.
#
#    ext=$(call run-pkg-config,--cflags,$(requires_all));
#
#    - under Mac OS/X, it work well in all rules
#    - under Linux, it can't work properly in depend rules
#    - under Linux, it can work well all other rules
#
#  Look like in make rule's recipe make function call can __ONLY__ work well
#  if they __DON'T__ execute any system commands but just __ONLY__ process
#  make variables.
#
#  In this case, pkg-config PC files may not be ready before start
#  make dependence, guess in Linux, this make function call was execute
#  immediately so they can't get any expected --cflags result, but it may defer
#  execute under Mac OS/X so they can work as expected.  NEED TO BE CONFIRED.

quiet_cmd_mkdep_cc	?= DEP     $(subst ./,,$(patsubst $(topdir)/%,%,$<))
      cmd_mkdep_cc 	?= set -e; mkdir -p $(DEPEND_DIR);		\
	ext=`if [ ! -z "$(USE_HOSTTOOLS)" ]; then			\
	       if [ -e $(PKG_CONFIG) ] && [ ! -z "$(requires_all)" ]; then \
	         $(PKGCONFIG_CMD) --cflags $(requires_all); fi; fi`;	\
	$(CC) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $$ext $(TARGET_ARCH) -M $< | \
	$(SED) 's,\($(notdir $*)\.$(OBJ_EXT)\) *:,$(OBJ_DIR)/\1 $(OBJ_DEBUG_DIR)/\1 $@: ,' > $@ 

$(DEPEND_DIR)/%.$(DEPEND_EXT): %.c
	$(call cmd,mkdep_cc)


#####
# compile rules...

quiet_cmd_cc		?= CC      $(subst ./,,$(patsubst $(topdir)/%,%,$<))
      cmd_cc		?= set -e; mkdir -p $(OBJ_DIR);			\
	$(CC) -c $(INCLUDES) $(CPPFLAGS) $(CFLAGS) -o $@ $< 

quiet_cmd_cc_d		?= CC      $(subst ./,,$(patsubst $(topdir)/%,%,$<)) (debug) 
      cmd_cc_d		?= set -e; mkdir -p $(OBJ_DEBUG_DIR);		\
	$(CC) -c $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $(CFLAGS_D) -o $@ $< 

$(OBJ_DIR)/%.$(OBJ_EXT): %.c
	$(call cmd,cc)

$(OBJ_DEBUG_DIR)/%.$(OBJ_EXT): %.c
	$(call cmd,cc_d)


#####
# library rules...

quiet_cmd_ar		?= AR      $$@
      cmd_ar		?= set -e; mkdir -p $(OBJ_DIR);			\
	$(AR) $(ARFLAGS) $$@ $$^ $$(call run-pkg-config,--libs,$$($$(patsubst lib%d,%,$$(call lib_name,$$(notdir $$@)))_requires))$(trash) 2>&1

quiet_cmd_ard		?= AR      $$@ (debug)
      cmd_ard		?= set -e; mkdir -p $(OBJ_DEBUG_DIR);		\
	$(AR) $(ARFLAGS) $$@ $$^ $$(call run-pkg-config,--libs,$$($$(patsubst lib%d,%,$$(call lib_name,$$(notdir $$@)))_requires))$(trash) 2>&1

quiet_cmd_shlib		?= LD      $$@
      cmd_shlib		?= set -e; mkdir -p $(OBJ_DIR);			\
	$(CC) -o $$@ $(LDFLAGS) $$(call shared_opts,$$(call rmpath,$(OBJ_DIR),$$@)) $$^ $$(call run-pkg-config,--libs,$$($$(patsubst lib%,%,$$(call lib_name,$$(notdir $$@)))_requires)); \
	(cd $(OBJ_DIR) && $(call shared_cmds,$$(call rmpath,$(OBJ_DIR),$$@)))

quiet_cmd_shlibd	?= LD      $$@ (debug)
      cmd_shlibd	?= set -e; mkdir -p $(OBJ_DEBUG_DIR);		\
	$(CC) -o $$@ $(LDFLAGS_D) $$(call shared_opts,$$(call rmpath,$(OBJ_DEBUG_DIR),$$@)) $$^ $$(call run-pkg-config,--libs,$$($$(patsubst lib%d,%,$$(call lib_name,$$(notdir $$@)))_requires)); \
	(cd $(OBJ_DEBUG_DIR) && $(call shared_cmds,$$(call rmpath,$(OBJ_DEBUG_DIR),$$@)))

quiet_cmd_symbolic	?= SYMLINK $$@ -> $$<
      cmd_symbolic	?= set -e; mkdir -p $(OBJ_DIR);			\
	(cd $(OBJ_DIR) && ln -sf $$(call soname,$$(call rmpath,$(OBJ_DIR),$$<)) $$(call rmpath,$(OBJ_DIR),$$@))

quiet_cmd_symbolicd	?= SYMLINK $$@ -> $$< (debug)
      cmd_symbolicd	?= set -e; mkdir -p $(OBJ_DEBUG_DIR);		\
	(cd $(OBJ_DEBUG_DIR) && ln -sf $$(call soname,$$(call rmpath,$(OBJ_DEBUG_DIR),$$<)) $$(call rmpath,$(OBJ_DEBUG_DIR),$$@))

# $(call make-library, pwd, name, version, srcs, deps)
define make-library
  vpath			%.c $(call make-vpath,$1,$4)
  INCLUDES		:= $(call add-incs,$(INCLUDES),$1,$4)

  libs_all		+= $2
  srcs_all		+= $(notdir $4)

  # setup all library modules dependence
  $2_deps		:= $5

  # required package process...
  requires_all		:= $(strip $(sort $(requires_all) $6))
  $2_requires		:= $6

  ifeq ($(LIBRARY),shared)
    $(OBJ_DIR)/lib$2.$(call shared_ext,$3):				\
      $(if $5,$(foreach t,$5,$(OBJ_DIR)/lib$t.$(LIB_EXT)),)		\
      $(addprefix $(OBJ_DIR)/,$(call srcs-to-objs,$(notdir $4)))
	$(call cmd,shlib)

    $(OBJ_DIR)/lib$2.$(LIB_EXT):					\
      $(OBJ_DIR)/lib$2.$(call shared_ext,$3)
	$(call cmd,symbolic)

    $(OBJ_DEBUG_DIR)/lib$2d.$(call shared_ext,$3):			\
      $(if $5,$(foreach t,$5,$(OBJ_DEBUG_DIR)/lib$td.$(LIB_EXT)),)	\
      $(addprefix $(OBJ_DEBUG_DIR)/,$(call srcs-to-objs,$(notdir $4)))
	$(call cmd,shlibd)

    $(OBJ_DEBUG_DIR)/lib$2d.$(LIB_EXT):					\
      $(OBJ_DEBUG_DIR)/lib$2d.$(call shared_ext,$3)
	$(call cmd,symbolicd)
  else
    ifeq ($(OPSYS),linux)
      $(OBJ_DIR)/lib$2.$(LIB_EXT):					\
        $(if $5,$(foreach t,$5,$(OBJ_DIR)/lib$t.$(LIB_EXT)),)		\
        $(addprefix $(OBJ_DIR)/,$(call srcs-to-objs,$(notdir $4)))
	  $(call cmd,ar)

      $(OBJ_DEBUG_DIR)/lib$2d.$(LIB_EXT):				\
        $(if $5,$(foreach t,$5,$(OBJ_DEBUG_DIR)/lib$td.$(LIB_EXT)),)	\
        $(addprefix $(OBJ_DEBUG_DIR)/,$(call srcs-to-objs,$(notdir $4)))
	  $(call cmd,ard)
    endif
  endif
endef


#####
# plugin rules...

# $(call set-rpath-otps $1(-L... -L...) $2(-l... -l..))
set-rpath-opts		= -Wl,-rpath,$(subst -L,,$(shell echo $1| sed -e 's/ /:/g')) \
			  $(strip					\
			    $(foreach t,$(subst -l,,$2),		\
			      $(foreach p,$(subst -L,,$1),		\
			        $(wildcard $p/lib$t.so))))


quiet_cmd_plugin	?= PLUGIN  $$@
      cmd_plugin	?= set -e; mkdir -p $(OBJ_DIR);			\
	$(CC) -o $$@ $(LDFLAGS)						\
	$$(call shared_opts,$$(call rmpath,$(OBJ_DIR),$$@))		\
	$$^								\
	$$(call set-rpath-opts,						\
	  $$(call run-pkg-config,--libs-only-L,$$($$(patsubst lib%,%,$$(call lib_name,$$(notdir $$@)))_requires)) -L$(abspath $(OBJ_DIR)),\
	  $$(call run-pkg-config,--libs-only-l,$$($$(patsubst lib%,%,$$(call lib_name,$$(notdir $$@)))_requires)) \
	  $$(patsubst %,-l%,$$($$(patsubst lib%,%,$$(call lib_name,$$(notdir $$@)))_deps)))

quiet_cmd_plugind	?= PLUGIN  $$@ (debug)
      cmd_plugind	?= set -e; mkdir -p $(OBJ_DEBUG_DIR);		\
	$(CC) -o $$@ $(LDFLAGS_D)					\
	$$(call shared_opts,$$(call rmpath,$(OBJ_DEBUG_DIR),$$@))	\
	$$^								\
	$$(call set-rpath-opts,						\
	  $$(call run-pkg-config,--libs-only-L,$$($$(patsubst lib%d,%,$$(call lib_name,$$(notdir $$@)))_requires)) -L$(abspath $(OBJ_DEBUG_DIR)),\
	  $$(call run-pkg-config,--libs-only-l,$$($$(patsubst lib%d,%,$$(call lib_name,$$(notdir $$@)))_requires)) \
	  $$(patsubst %,-l%d,$$($$(patsubst lib%d,%,$$(call lib_name,$$(notdir $$@)))_deps)))

# $(call make-plugin, pwd, name, version, srcs, deps)
define make-plugin
  vpath			%.c $(call make-vpath,$1,$4)
  INCLUDES		:= $(call add-incs,$(INCLUDES),$1,$4)

  plugins_all		+= $2
  srcs_all		+= $(notdir $4)

  # setup all library modules dependence
  $2_deps		:= $5

  # required package process...
  requires_all		:= $(strip $(sort $(requires_all) $6))
  $2_requires		:= $6

  $(OBJ_DIR)/lib$2.$(LIB_EXT):						\
    $(if $5,$(foreach t,$5,$(OBJ_DIR)/lib$t.$(LIB_EXT)),)		\
    $(addprefix $(OBJ_DIR)/,$(call srcs-to-objs,$(notdir $4)))
	$(call cmd,plugin)

  $(OBJ_DEBUG_DIR)/lib$2d.$(LIB_EXT):					\
    $(if $5,$(foreach t,$5,$(OBJ_DEBUG_DIR)/lib$td.$(LIB_EXT)),)	\
    $(addprefix $(OBJ_DEBUG_DIR)/,$(call srcs-to-objs,$(notdir $4)))
	$(call cmd,plugind)
endef


#####
# app rules...
#

quiet_cmd_app		?= LD      $$@
      cmd_app		?= set -e; mkdir -p $(OBJ_DIR);			\
	$(CC) $(CFLAGS) -o $$@ $$(filter-out %.$(LIB_EXT),$$^) $$(call xfer-libs,$(OBJ_DIR),$$^,$(LIB_EXT)) $$(call run-pkg-config,--libs,$$($$(notdir $$@)_requires)) $(filter -l%,$(LDFLAGS))

quiet_cmd_appd		?= LD      $$@ (debug)
      cmd_appd		?= set -e; mkdir -p $(OBJ_DEBUG_DIR);		\
	$(CC) $(CFLAGS) $(CFLAGS_D) -o $$@ $$(filter-out %.$(LIB_EXT),$$^) $$(call xfer-libs,$(OBJ_DEBUG_DIR),$$^,$(LIB_EXT)) $$(call run-pkg-config,--libs,$$($$(patsubst %d,%,$$(notdir $$@))_requires)) $(filter -l%,$(LDFLAGS))

# $(call make-application, pwd, name, version, srcs, deps, requires)
define make-application
  vpath			%.c $(call make-vpath,$1,$4)
  INCLUDES		:= $(call add-incs,$(INCLUDES),$1,$4)

  apps_all		+= $2
  srcs_all		+= $(notdir $(call filter-srcs,$4))

  # required package process...
  requires_all		:= $(strip $(sort $(requires_all) $6))
  $2_requires		:= $6

  $(OBJ_DIR)/$2:							\
    $(patsubst %,$(OBJ_DIR)/lib%.$(LIB_EXT),$5 $(call all_rdeps,$5))	\
    $(addprefix $(OBJ_DIR)/,$(call srcs-to-objs,$(notdir $4)))		\
    $(addprefix $(OBJ_DIR)/,$(notdir $(filter %.a,$4)))			\
    $(addprefix $(OBJ_DIR)/,$(notdir $(filter %.o,$4)))
	$(call cmd,app)

  $(OBJ_DEBUG_DIR)/$2d:							\
    $(patsubst %,$(OBJ_DEBUG_DIR)/lib%d.$(LIB_EXT),$5 $(call all_rdeps,$5)) \
    $(addprefix $(OBJ_DEBUG_DIR)/,$(call srcs-to-objs,$(notdir $4)))	\
    $(addprefix $(OBJ_DEBUG_DIR)/,$(notdir $(filter %.a,$4)))		\
    $(addprefix $(OBJ_DEBUG_DIR)/,$(notdir $(filter %.o,$4)))
	$(call cmd,appd)
endef


#
# wrap all modules together...
#

include $(patsubst %,%/module.mk,$(modules))

#
# add all depends...
#

objs_all		= $(call srcs-to-objs,$(srcs_all))
depends			:= $(patsubst %,$(DEPEND_DIR)/%.$(DEPEND_EXT),	\
			     $(patsubst %.$(OBJ_EXT),%,$(objs_all)))
exclude_targets		:= $(EXTRA_EXCLUDE_TARGETS) 			\
			   $(depends_exclude_targets)			\
			   clean distclean info.targets info.debug rdep_test help

ifneq ($(USE_HOSTTOOLS),)
  ifeq ($(filter $(exclude_targets),$(MAKECMDGOALS)),)
    # TODO: find a way to build tools automatically.
    $(if $(wildcard $(PKG_CONFIG)),,					\
      $(error Oops, pkg-config not installed, use "make env" firstly.))
  endif
endif

ifeq ($(filter $(exclude_targets),$(MAKECMDGOALS)),)
  ifeq ($(USE_VERSION.H),yes)
    $(DEPEND_DIR)/$(patsubst %.c,%.$(DEPEND_EXT),$(word $(words $(srcs_all)),$(srcs_all))): $(builddir)/version.h
  endif
  -include $(depends)
endif 

# add extra package's include directory into CFLAGS
# NOTE:
#   We must put this line behind "-include $(depends)" because
#   neet to wait pkg-config was built.
#
CFLAGS			+= $(call run-pkg-config,--cflags,$(requires_all))

#
# main targets ...
#

rel: $(libs_all:%=$(OBJ_DIR)/lib%.$(LIB_EXT)) $(plugins_all:%=$(OBJ_DIR)/lib%.$(LIB_EXT)) $(apps_all:%=$(OBJ_DIR)/%)
debug: $(libs_all:%=$(OBJ_DEBUG_DIR)/lib%d.$(LIB_EXT)) $(plugins_all:%=$(OBJ_DEBUG_DIR)/lib%d.$(LIB_EXT)) $(apps_all:%=$(OBJ_DEBUG_DIR)/%d)
profile:


my_clean:
	@$(RM) -fr $(DEPEND_DIR) $(OBJ_DIR) $(OBJ_DEBUG_DIR) $(EXTRA_CLEAN)

clean: my_clean

#
# .gdbinit
#

ifneq ($(wildcard .gdbinit),)
define filechk_$(OBJ_DEBUG_DIR)/.gdbinit
	(sed -e 's/%%/$(if $(findstring osx,$(OPSYS)),DY)/g';)
endef

$(OBJ_DEBUG_DIR)/.gdbinit: .gdbinit Makefile FORCE
	$(call filechk,$(OBJ_DEBUG_DIR)/.gdbinit)

debug: $(OBJ_DEBUG_DIR)/.gdbinit
endif

.PHONY: FORCE
FORCE:

#
# test stuff ...
#

info.targets:
	@echo
	@echo "  Libraries format: $(LIBRARY)"
	@echo
	@echo "  Available targets:"
	@echo "  ------------------"
	@for t in $(libs_all:%=$(OBJ_DIR)/lib%.$(LIB_EXT)); do 		\
	    echo "  $$t"; 						\
	done
	@for t in $(libs_all:%=$(OBJ_DEBUG_DIR)/lib%d.$(LIB_EXT)); do	\
	    echo "  $$t"; 						\
	done
	@for t in $(plugins_all:%=$(OBJ_DIR)/lib%.$(LIB_EXT)); do 	\
	    echo "  $$t"; 						\
	done
	@for t in $(plugins_all:%=$(OBJ_DEBUG_DIR)/lib%d.$(LIB_EXT)); do\
	    echo "  $$t"; 						\
	done
	@for t in $(apps_all:%=$(OBJ_DIR)/%); do 			\
	    echo "  $$t"; 						\
	done
	@for t in $(apps_all:%=$(OBJ_DEBUG_DIR)/%d); do			\
	    echo "  $$t"; 						\
	done

info.debug:
	@echo "CC = $(CC)"
	@echo "CPPFLAGS = $(CPPFLAGS)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "apps = $(apps)"
	@echo "objs_all = $(objs_all)"
	@echo "libs_all = $(libs_all)"
	@echo "plugins_all = $(plugins_all)"
	@echo "depends = $(depends)"

a_deps			= b d x
b_deps			= c
d_deps			= y
f_deps			= x
g_deps			= b

rdep_test:
	@echo "a m_rdeps=" $(call m_rdeps,$(a_deps)) 
	@echo "a all_rdeps=" $(call all_rdeps,$(a_deps))

