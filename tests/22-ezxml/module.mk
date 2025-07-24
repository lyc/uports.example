pwd			:= $(call subdirectory,module.mk)

name			:= t-xml
version			:= 0.01.0
srcs			:= t-xml.c
deps			:= utils
requires		:= ezxml


$(eval									\
  $(call make-application,$(pwd),$(name),$(version),$(srcs),$(deps),$(requires)))
