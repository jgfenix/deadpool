
#include <Elementary.h>

static Evas_Object *in_win ;

static void//exit
  exit_program(void *data, Evas_Object *obj, void *event_info)
  {
     elm_exit();
  }

static void
    back_win(void *data, Evas_Object *obj, void *event)
    {
       evas_object_del(in_win);
    }

static void//inwin mode 'on' to show the content of app button
   b_inw_cb(void *data, Evas_Object *previous_win, void *event_info)
   {
    
    Evas_Object  *win, *inw_box, *ic, *b_back, *bt ;
    
    win = elm_object_top_widget_get(previous_win);//get the previous window as parameter
      
    in_win = elm_win_inwin_add(win); 
    elm_win_inwin_activate(in_win);//or evas_object_show(in_win);
       
    inw_box = elm_box_add(win);
    elm_box_horizontal_set(inw_box, EINA_TRUE);
    evas_object_size_hint_weight_set(inw_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); 
    elm_win_resize_object_add(win, inw_box);
    elm_box_padding_set(inw_box, 1, 1);
    evas_object_show(inw_box);

    b_back = elm_button_add(win);
    elm_object_text_set(b_back, " back ");
    elm_box_pack_end(inw_box, b_back);
    evas_object_show(b_back);
    evas_object_smart_callback_add(b_back, "clicked", back_win, NULL);
        
    ic = elm_icon_add(win);
    elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_THEME_FDO);
    elm_icon_standard_set(ic, "chat");
    evas_object_show(ic);

    bt = elm_button_add(win);
    elm_object_text_set(bt, "chat ");
    elm_object_part_content_set(bt, "icon", ic);
    evas_object_size_hint_min_set(bt, 80, 50); 
    elm_box_pack_end(inw_box, bt);
    evas_object_show(bt);
 
    ic = elm_icon_add(win);
    elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_THEME_FDO);
    elm_icon_standard_set(ic, "media_player/play");
    evas_object_show(ic);

    bt = elm_button_add(win);
    elm_object_text_set(bt, "Player ");
    elm_object_part_content_set(bt, "icon", ic);
    evas_object_size_hint_min_set(bt, 80, 50); 
    elm_box_pack_end(inw_box, bt);
    evas_object_show(bt);
           
    elm_win_inwin_content_set(in_win, inw_box);  
   }

//if you select an image, this callback shows it in another window and 
//expands to show the entire image
static void
   _file_chosen(void *data, Evas_Object *obj, void *selected_item)
  {  
   Evas_Object *win, *image;
    char *file = selected_item;  
    int w, h, hg;

   if (file != NULL)
     {   
        win = elm_win_util_standard_add("image", "Image");
        elm_win_autodel_set(win, EINA_TRUE); 
        image = elm_image_add(win);
          
        if (!elm_image_file_set(image, file, NULL))
         {
           printf("error: could not load image \"%s\"\n", file);
         }
 
        elm_image_object_size_get(image, &w, &h);
                
        hg = w;//used in elm_image_prescale_set to make the right scale and show without low pixels
        if(hg < h) hg = h;
        
        if( w != 0 && h != 0)//if the width and height are greater than 0, make a window to show the image
         {
           printf("\nImage path: %s", file); 
           printf("\nwidth x height = %d x %d\n", w, h);
           
           elm_image_no_scale_set(image, EINA_FALSE);
           elm_image_resizable_set(image, EINA_TRUE, EINA_TRUE);
           elm_image_smooth_set(image, EINA_TRUE);
           elm_image_orient_set(image, ELM_IMAGE_ORIENT_NONE);
           elm_image_aspect_fixed_set(image, EINA_FALSE);
           elm_image_fill_outside_set(image, EINA_TRUE);
           elm_image_editable_set(image, EINA_TRUE);
           
           elm_image_prescale_set(image, hg);  
           elm_win_resize_object_add(win, image);
           evas_object_size_hint_weight_set(image, EVAS_HINT_FILL, EVAS_HINT_FILL);
           evas_object_resize(win, 200, 200);
           evas_object_show(win);
           evas_object_show(image);
         }
        else
          printf("* Invalid path or image *\n");

     }
   else
     printf("File selection canceled.\n");
  }
  
static void //a window that you can write
  text_set(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *win, *btn, *bx, *en;

   win = elm_win_util_standard_add("window_to_write_button", "Write");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_object_text_set(en,
                       "This is a multi-line entry at the bottom<br>"
                       "This can contain more than 1 line of text and be "
                       "scrolled around to allow for entering of lots of "
                       "content. It is also to test to see that autoscroll "
                       "moves to the right part of a larger multi-line "
                       "text entry that is inside of a scroller than can be "
                       "scrolled around, thus changing the expected position "
                       "as well as cursor changes updating auto-scroll when "
                       "it is enabled.");

   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "no_fuction_button");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0);
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   evas_object_resize(win, 240, 480);
   evas_object_show(win);
}

static void//second naviframe 
  n_frame_2(void *data, Evas_Object *obj, void *event_info)
  {
   Evas_Object *nv = data, *ic, *bt, *nv_box, *b_exit;
   
  if(nv != NULL)
   { 
   nv_box = elm_box_add(nv);
   elm_box_horizontal_set(nv_box, EINA_FALSE);
   evas_object_size_hint_weight_set(nv_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); 
   elm_win_resize_object_add(nv, nv_box);
   evas_object_show(nv_box);

   ic = elm_icon_add(nv);
   elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_THEME_FDO);
   elm_icon_standard_set(ic, "edit");
   evas_object_show(ic);
  
   bt = elm_button_add(nv);
   elm_object_text_set(bt, "Write");
   elm_object_part_content_set(bt, "icon", ic);
   elm_box_pack_end(nv_box, bt);
   evas_object_size_hint_min_set(bt, 280, 280);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", text_set, NULL);

   b_exit = elm_button_add(nv);
   elm_object_text_set(b_exit, " EXIT ");
   elm_box_pack_end(nv_box, b_exit);
   evas_object_show(b_exit);
   evas_object_smart_callback_add(b_exit, "clicked", exit_program, NULL);

  elm_naviframe_item_push(nv, NULL, NULL, NULL, nv_box, NULL);
   }
  }

static void//'first' naviframe mode
  n_frame_1(void *data, Evas_Object *obj, void *event_info)
  {
   Evas_Object *nv = data, *nv_box, *ic, *fs_bt, *en, *b_next, *b_exit;
   
  if(nv != NULL)
   {
   nv_box = elm_box_add(nv);
   elm_box_horizontal_set(nv_box, EINA_FALSE);
   evas_object_size_hint_weight_set(nv_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); 
   elm_win_resize_object_add(nv, nv_box);
   evas_object_show(nv_box);

   ic = elm_icon_add(nv);
   elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_THEME_FDO);
   elm_icon_standard_set(ic, "menu/folder");
   evas_object_show(ic);
  
   fs_bt = elm_fileselector_button_add(nv);
   elm_fileselector_button_path_set(fs_bt, "/home");
   elm_object_text_set(fs_bt, "Select a file");
   elm_object_part_content_set(fs_bt, "icon", ic);
   elm_fileselector_button_inwin_mode_set(fs_bt, EINA_TRUE);
   elm_box_pack_end(nv_box, fs_bt);
   evas_object_size_hint_min_set(fs_bt, 280, 280);
   evas_object_show(fs_bt);
   
   en = elm_entry_add(nv);
   elm_entry_line_wrap_set(en, EINA_FALSE);
   elm_entry_editable_set(en, EINA_FALSE);
   evas_object_smart_callback_add(fs_bt, "file,chosen", _file_chosen, en);
   elm_box_pack_end(nv_box, en);
   evas_object_show(en);

   ic = elm_icon_add(nv);
   elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_THEME_FDO);
   elm_icon_standard_set(ic, "arrow_right");
   evas_object_show(ic);

   b_next = elm_button_add(nv);
   elm_object_text_set(b_next, "Next");
   elm_object_part_content_set(b_next, "icon",ic);
   elm_box_pack_end(nv_box, b_next);
   evas_object_size_hint_min_set(b_next, 60, 40); 
   evas_object_show(b_next);
   evas_object_smart_callback_add(b_next, "clicked", n_frame_2, nv);
 
   b_exit = elm_button_add(nv);
   elm_object_text_set(b_exit, " EXIT ");
   elm_box_pack_end(nv_box, b_exit);
   evas_object_show(b_exit);
   evas_object_smart_callback_add(b_exit, "clicked", exit_program, NULL);
    
   elm_naviframe_item_push(nv, NULL, NULL, b_next, nv_box, NULL);
   //elm_naviframe_item_push(nv, NULL, NULL, NULL, fs_bt, NULL);//expand the icon if you don't pack fs_bt
   }
  }


EAPI_MAIN int

elm_main(int argc, char **argv)
{
   Evas_Object *win, *nv, *box, *app_box, *_clock, *ic, *b_next, *b_exit, *b_app, *image ;
  
   char buf[PATH_MAX] = "/usr/local/share/elementary/images/logo.png";

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);  

   win = elm_win_util_standard_add("WIN", "_naviframe_");
   evas_object_smart_callback_add(win, "delete,request", exit_program, NULL);
   elm_win_autodel_set(win, EINA_TRUE); 
   evas_object_resize(win, 600, 500);
   evas_object_show(win);
   
   nv = elm_naviframe_add(win);
   evas_object_size_hint_weight_set(nv, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, nv);
   evas_object_show(nv);  
   
   box = elm_box_add(win);
   elm_box_horizontal_set(box, EINA_TRUE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); 
   elm_win_resize_object_add(nv, box);
   evas_object_show(box);
 
   _clock = elm_clock_add(nv);
   elm_clock_show_am_pm_set(_clock, EINA_TRUE);
   elm_box_pack_start(box, _clock);
   evas_object_show(_clock);
   
   app_box = elm_box_add(nv);
   elm_box_horizontal_set(app_box, EINA_TRUE);
   elm_box_homogeneous_set(app_box, EINA_TRUE);
   evas_object_size_hint_weight_set(app_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); 
   elm_win_resize_object_add(nv, app_box);
   evas_object_show(app_box);

   ic = elm_icon_add(nv);
   elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_THEME_FDO);
   elm_icon_standard_set(ic, "arrow_right");
   evas_object_show(ic);

   b_next = elm_button_add(nv);
   elm_object_text_set(b_next, "Next");
   elm_object_part_content_set(b_next, "icon",ic);
   elm_box_pack_end(box, b_next);
   evas_object_size_hint_min_set(b_next, 60, 40); 
   evas_object_show(b_next);
   evas_object_smart_callback_add(b_next, "clicked", n_frame_1, nv);
   
   b_exit = elm_button_add(nv);
   elm_object_text_set(b_exit, " EXIT ");
   elm_box_pack_start(box, b_exit);
   evas_object_size_hint_min_set(b_exit, 60, 40); 
   evas_object_show(b_exit);
   evas_object_smart_callback_add(b_exit, "clicked", exit_program, NULL);

   ic = elm_icon_add(nv);
   elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_THEME_FDO);
   elm_icon_standard_set(ic, "apps");
   evas_object_show(ic);

  image = elm_image_add(win);
  
  if (!elm_image_file_set(image, buf, NULL))
     {
        printf("error: could not load image \"%s\"\n", buf);
        return -1;
     }

   elm_image_no_scale_set(image, EINA_TRUE);
   elm_image_resizable_set(image, EINA_FALSE, EINA_TRUE);
   elm_image_smooth_set(image, EINA_FALSE);
   elm_image_orient_set(image, ELM_IMAGE_ORIENT_NONE);
   elm_image_aspect_fixed_set(image, EINA_TRUE);
   elm_image_fill_outside_set(image, EINA_FALSE);
   elm_image_editable_set(image, EINA_TRUE);
   evas_object_show(image);

   b_app = elm_button_add(nv);
   elm_object_text_set(b_app, " CLICK ME");
   elm_object_part_content_set(b_app,"icon", image);
   elm_box_pack_end(app_box, b_app);
   evas_object_show(b_app);
   elm_win_resize_object_add(b_app, image);
   evas_object_smart_callback_add(b_app, "clicked", b_inw_cb, win);
  
   elm_naviframe_item_push(nv, NULL, NULL, box, app_box, NULL);//'zero' naviframe mode
   
   elm_run();
   elm_shutdown();
  
 return 0;
}
 ELM_MAIN()


