pwd			:= $(call subdirectory,module.mk)

version			:= 0.01.0
srcs			:=
deps			:= utils
requires		:=

#name			:=
names_all		:= t-cport					\
			   t-tpool

#
# create application targets ...
#

$(foreach t,$(names_all),						\
  $(eval								\
    $(call make-application,						\
      $(pwd),$t,$(version),$t.c $(srcs),$(deps),$(requires))))
