pwd			:= $(call subdirectory,module.mk)

name			:= t-json
version			:= 0.01.0
srcs			:= t-json.c
deps			:=
requires		:= json-c

$(eval									\
  $(call make-application,$(pwd),$(name),$(version),$(srcs),$(deps),$(requires)))
