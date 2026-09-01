pwd			:= $(call subdirectory,module.mk)

name			:= utils
version			:= 0.0.1

src-y			:= tree.c                                       \
			   value_base.c					\
			   value_types.c				\
			   value_codec.c				\
			   stack.c					\
			   thread.c
dep-y			:=
req-y			:=

$(eval									\
  $(call make-library,$(pwd),$(name),$(version),$(src-y),$(dep-y),$(req-y)))
