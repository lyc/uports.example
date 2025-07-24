pwd			:= $(call subdirectory,module.mk)

version			:= 0.01.0
name			:= t-values
srcs			:= t-values.c
deps			:= utils
requires		:=

$(eval									\
  $(call make-application,						\
    $(pwd),$(name),$(version),$(srcs),$(deps),$(requires)))
