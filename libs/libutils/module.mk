pwd			:= $(call subdirectory,module.mk)

name			:= utils
version			:= 0.0.1

src-y			:= thread.c					\
			   tree.c
dep-y			:=
req-y			:=

$(eval									\
  $(call make-library,$(pwd),$(name),$(version),$(src-y),$(dep-y),$(req-y)))
