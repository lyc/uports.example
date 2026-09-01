pwd			:= $(call subdirectory,module.mk)

version			:= 0.01.0
deps			:= utils
requires		:=

# Focused verification programs for the libutils value framework.
names_app		:= t_value_list				\
			   t_value_domain_key			\
			   t_value_no_key			\
			   t_value_metadata			\
			   t_value_embedded			\
			   t_value_list_mutation		\
			   t_value_errors			\
			   t_value_types			\
			   t_value_macros			\
			   t_value_filter_ref			\
			   t_value_codec			\
			   t_ru_value_model

t_ru_value_model_srcs	:= t_ru_value_fixture.c t_ru_scenarios.c

$(foreach t,$(names_app),						\
  $(eval								\
    $(call make-application,						\
      $(pwd),$(t),$(version),$(t).c $($(t)_srcs),$(deps),$(requires))))
