#include <Elementary.h>

static Evas_Object *in_win ;


static void
  exit_cb(void *data, Evas_Object *obj, void *event_info)
  {
          elm_exit();
  }


static void
    back_win(void *data, Evas_Object *obj, void *event)
    {
       evas_object_del(in_win);
    }
 

static void
   b_inw_cb(void *data, Evas_Object *previous_win, void *event_info)
   {
    
    Evas_Object *_win, *in_box, *fs_bt, *ic, *ic_b, *b_back ;
    
    _win = elm_object_top_widget_get(previous_win);//get the previous window as parameter
   
       
    in_win = elm_win_inwin_add(_win); 
    evas_object_show(in_win);//elm_win_inwin_activate(in_win);
   
        
    in_box = elm_box_add(_win);
    elm_box_horizontal_set(in_box, EINA_TRUE);
    evas_object_size_hint_weight_set(in_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); 
    elm_win_resize_object_add(_win, in_box);
    elm_box_padding_set(in_box, 1, 1);
    evas_object_show(in_box);


    ic = elm_icon_add(_win);
    elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_THEME_FDO);
    elm_icon_standard_set(ic, "menu/folder");
    evas_object_show(ic);
  

    ic_b = elm_icon_add(_win);
    elm_icon_order_lookup_set(ic_b, ELM_ICON_LOOKUP_THEME_FDO);
    elm_icon_standard_set(ic_b, "arrow_left");
    evas_object_show(ic_b);
     

    fs_bt = elm_fileselector_button_add(_win);
    elm_fileselector_button_path_set(fs_bt, "/home");
    elm_object_text_set(fs_bt, "Select a file");
    elm_object_part_content_set(fs_bt, "icon", ic);
    elm_box_pack_end(in_box, fs_bt);
    elm_fileselector_button_inwin_mode_set(fs_bt, EINA_TRUE);
    evas_object_show(fs_bt);

    
    b_back = elm_button_add(in_box);
    elm_object_text_set(b_back, "return ");
    elm_object_part_content_set(b_back, "icon", ic_b);
    elm_box_pack_start(in_box, b_back);
    evas_object_show(b_back);
    evas_object_smart_callback_add(b_back, "clicked", back_win, NULL);
    elm_win_inwin_content_set(in_win, in_box);  
   }

EAPI_MAIN int

elm_main(int argc, char **argv)
{
   Evas_Object *win, *box, *b_inw, *b_exit ;
   
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);  


    win = elm_win_util_standard_add("reader", "Reader");
    evas_object_smart_callback_add(win, "delete,request", exit_cb, NULL);
    elm_win_autodel_set(win, EINA_TRUE);
    evas_object_resize(win, 400, 300);
    evas_object_show(win);
   
    
    box = elm_box_add(win);
    elm_box_horizontal_set(box, EINA_TRUE);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); 
    elm_win_resize_object_add(win, box);
    elm_box_padding_set(box, 1, 1);
    elm_box_homogeneous_set(box,EINA_TRUE);
    evas_object_show(box);

    
    b_inw = elm_button_add(win);
    elm_object_text_set(b_inw, "  click me  ");
    elm_box_pack_end(box, b_inw);
    evas_object_show(b_inw);
    evas_object_smart_callback_add(b_inw, "clicked", b_inw_cb, NULL);
    
   
    b_exit = elm_button_add(win);
    elm_object_text_set(b_exit, "  EXIT  ");
    elm_box_pack_end(box, b_exit);
    evas_object_show(b_exit);
    evas_object_smart_callback_add(b_exit, "clicked", exit_cb, NULL);
   

   
   elm_run();
   elm_shutdown();
  
 return 0;
}
 ELM_MAIN()

