#include <Elementary.h>

static void
  on_done(void *data, Evas_Object *obj, void *event_info)
  {
          elm_exit();
  }

void
  n_frame(void *data, Evas_Object *obj, void *event_info)
  {
   Evas_Object *nv = data, *b_n, *box, *ic, *fs_bt;
   

   box = elm_box_add(nv);
   elm_box_horizontal_set(box, EINA_TRUE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); 
   elm_win_resize_object_add(nv, box);
   evas_object_show(box);

   ic = elm_icon_add(nv);
   elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_THEME_FDO);
   elm_icon_standard_set(ic, "menu/folder");
   evas_object_show(ic);
  
   fs_bt = elm_fileselector_button_add(nv);
   elm_fileselector_button_path_set(fs_bt, "/home");
   elm_object_text_set(fs_bt, "Select a file");
   elm_object_part_content_set(fs_bt, "icon", ic);
   elm_fileselector_button_inwin_mode_set(fs_bt, EINA_TRUE);
   elm_box_pack_end(box, fs_bt);
   evas_object_show(fs_bt);

   elm_naviframe_item_push(nv, NULL, NULL, NULL, box, NULL);
   //elm_naviframe_item_push(nv, NULL, NULL, NULL, fs_bt, NULL);//expand the icon if you don't pack fs_bt
  }

EAPI_MAIN int

elm_main(int argc, char **argv)
{
   Evas_Object *win, *nv, *box, *ic, *fs_bt, *b_n, *b_exit ;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);  

   win = elm_win_util_standard_add("WIN_", "menu_test");
   evas_object_smart_callback_add(win, "delete,request", on_done, NULL);
   elm_win_autodel_set(win, EINA_TRUE); 
   evas_object_resize(win, 400, 300);
   evas_object_show(win);
   
   nv = elm_naviframe_add(win);
   evas_object_size_hint_weight_set(nv, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, nv);
   evas_object_show(nv);  

   box = elm_box_add(nv);
   elm_box_horizontal_set(box, EINA_TRUE);
   elm_box_homogeneous_set(box, EINA_TRUE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); 
   elm_win_resize_object_add(nv, box);
   evas_object_show(box);
 
   ic = elm_icon_add(nv);
   elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_THEME_FDO);
   elm_icon_standard_set(ic, "arrow_right");
   evas_object_show(ic);

   b_n = elm_button_add(nv);
   elm_object_text_set(b_n, "Menu");
   elm_object_part_content_set(b_n, "icon",ic);
   elm_box_pack_end(box, b_n);
   evas_object_size_hint_min_set(b_n, 60, 40); 
   evas_object_show(b_n);
   evas_object_smart_callback_add(b_n, "clicked", n_frame, nv);
   
   b_exit = elm_button_add(nv);
   elm_object_text_set(b_exit, " EXIT ");
   elm_box_pack_start(box, b_exit);
   evas_object_size_hint_min_set(b_exit, 60, 40); 
   evas_object_show(b_exit);
   evas_object_smart_callback_add(b_exit, "clicked", on_done, NULL); 
  
   elm_naviframe_item_push(nv, NULL, NULL, box, NULL, NULL);
   
   elm_run();
   elm_shutdown();
  
 return 0;
}
 ELM_MAIN()

