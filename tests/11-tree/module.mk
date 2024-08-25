pwd			:= $(call subdirectory,module.mk)

version			:= 0.01.0
name			:= t-tree
srcs			:= t-tree.c
deps			:= utils
requires		:=

$(eval									\
  $(call make-application,						\
    $(pwd),$(name),$(version),$(srcs),$(deps),$(requires)))
