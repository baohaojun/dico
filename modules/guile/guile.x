/* cpp arguments: guile.c -DHAVE_CONFIG_H -I. -I../.. -I../../include -I../../include -I../../grecs/src -I/usr/local/include -pthread -I/usr/local/include -pthread -ggdb -Wall */
 scm_c_define_gsubr (s_scm_dico_key_p, 1, 0, 0, (SCM (*)()) scm_dico_key_p); scm_c_export (s_scm_dico_key_p, ((void *)0)); ;
 scm_c_define_gsubr (s_scm_dico_key__word, 1, 0, 0, (SCM (*)()) scm_dico_key__word); scm_c_export (s_scm_dico_key__word, ((void *)0)); ;
 scm_c_define_gsubr (s_scm_dico_strat_selector_p, 1, 0, 0, (SCM (*)()) scm_dico_strat_selector_p); scm_c_export (s_scm_dico_strat_selector_p, ((void *)0)); ;
 scm_c_define_gsubr (s_scm_dico_strat_select_p, 3, 0, 0, (SCM (*)()) scm_dico_strat_select_p); scm_c_export (s_scm_dico_strat_select_p, ((void *)0)); ;
 scm_c_define_gsubr (s_scm_dico_strat_name, 1, 0, 0, (SCM (*)()) scm_dico_strat_name); scm_c_export (s_scm_dico_strat_name, ((void *)0)); ;
 scm_c_define_gsubr (s_scm_dico_strat_description, 1, 0, 0, (SCM (*)()) scm_dico_strat_description); scm_c_export (s_scm_dico_strat_description, ((void *)0)); ;
 scm_c_define_gsubr (s_scm_dico_strat_default_p, 1, 0, 0, (SCM (*)()) scm_dico_strat_default_p); scm_c_export (s_scm_dico_strat_default_p, ((void *)0)); ;
 scm_c_define_gsubr (s_scm_dico_make_key, 2, 0, 0, (SCM (*)()) scm_dico_make_key); scm_c_export (s_scm_dico_make_key, ((void *)0)); ;
 scm_c_define_gsubr (s_scm_dico_register_strat, 2, 1, 0, (SCM (*)()) scm_dico_register_strat); scm_c_export (s_scm_dico_register_strat, ((void *)0)); ;
 scm_c_define_gsubr (s_scm_dico_register_markup, 1, 0, 0, (SCM (*)()) scm_dico_register_markup); scm_c_export (s_scm_dico_register_markup, ((void *)0)); ;
 scm_c_define_gsubr (s_scm_dico_current_markup, 0, 0, 0, (SCM (*)()) scm_dico_current_markup); scm_c_export (s_scm_dico_current_markup, ((void *)0)); ;
