pwd			:= $(call subdirectory,module.mk)

# FIXME: save pwd which may change inside target action
mycli			:= $(pwd)

version			:= 0.01.0
deps			:= utils
reqs			:=

#name			:=
names_all		:= ts

#
# extra requires & srcs ...
#

ts_reqs			:= json-c
ts_srcs			:=

#
# create application targets ...
#

$(foreach t,$(names_all),						\
  $(eval								\
    $(call make-application,						\
      $(pwd),$t,$(version),$t.c $($t_srcs),$(deps),$(reqs) $($t_reqs))))
