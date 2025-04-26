pwd			:= $(call subdirectory,module.mk)

#name			:= lt
version			:= 0.0.1
#srcs			:= lt.c hash.c
deps			:= utils 
requires		:= expat

name			:= outline
srcs			:= outline.c
$(eval									\
  $(call make-application,$(pwd),$(name),$(version),$(srcs),$(deps),$(requires)))

name			:= line
srcs			:= line.c
$(eval									\
  $(call make-application,$(pwd),$(name),$(version),$(srcs),$(deps),$(requires)))

name			:= namespace
srcs			:= namespace.c
$(eval									\
  $(call make-application,$(pwd),$(name),$(version),$(srcs),$(deps),$(requires)))
