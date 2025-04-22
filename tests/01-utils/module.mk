pwd			:= $(call subdirectory,module.mk)

version			:= 0.01.0
deps			:=
requires		:=


#name			:=
names_all		:= lt0-linux					\
			   lt1-list					\
			   lt2-hlist


#
# extra requires & srcs ...
#

lt2-hlist_srcs		:= hash.c

#
# create application targets ...
#

$(foreach t,$(names_all),						\
  $(eval								\
    $(call make-application,						\
      $(pwd),$t,$(version),$t.c $($t_srcs),$(deps),$(requires))))
