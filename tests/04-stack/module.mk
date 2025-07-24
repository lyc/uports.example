pwd			:= $(call subdirectory,module.mk)

name			:= t-stack
version			:= 0.0.1
srcs			:= t-stack.c
deps			:= utils
requires		:=

$(eval									\
  $(call make-application,$(pwd),$(name),$(version),$(srcs),$(deps),$(requires)))
