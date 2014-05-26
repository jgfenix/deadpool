#include <Elementary.h>

char name[PATH_MAX];

static void
  exit_cb(void *data, Evas_Object *obj, void *event_info)
  {
          elm_exit();
  }

 static void
  exit_th_win(void *data, Evas_Object *obj, void *event_info)
  {
          evas_object_del(event_info);
  }

static void
_generation_started_cb(void *data, Evas_Object *o, void *event_info)
{
   printf("thumbnail %s generation started.\n", name);
}

static void
_generation_finished_cb(void *data, Evas_Object *o, void *event_info)
{
   printf("thumbnail %s generation finished.\n", name);
}

static void
_generation_error_cb(void *data, Evas_Object *o, void *event_info)
{
   printf("thumbnail %s generation error.\n", name);
}
 
static void list (char *folder)//list path from file_chosen if it's a path
{   
   
   Evas_Object *th_win, *th_box, *thumb ;
   char buf[PATH_MAX];
   DIR *dir;
   struct dirent *lsdir; 
 
   elm_need_ethumb();

      
  if(opendir(folder) != NULL)
   {
     dir = opendir(folder);
     while ( ( lsdir = readdir(dir) ) != NULL )
      {
         if(strcmp(lsdir->d_name, ".") == 0 || strcmp(lsdir->d_name, "..") == 0  )
          continue;

        snprintf(name, sizeof(name), "%s", lsdir->d_name );


        printf("**%s\n",name);
        th_win = elm_win_util_standard_add("image_file", lsdir->d_name);
   evas_object_smart_callback_add(th_win, "delete,request", exit_th_win, NULL);
   elm_win_autodel_set(th_win, EINA_TRUE); 
   evas_object_resize(th_win, 150, 150);
   evas_object_show(th_win);   
  
   th_box = elm_box_add(th_win);
   elm_box_horizontal_set(th_box, EINA_FALSE);
   evas_object_size_hint_weight_set(th_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); 
   elm_win_resize_object_add(th_win, th_box);

        thumb = elm_thumb_add(th_win);
         
        evas_object_smart_callback_add(thumb, "generate,start", _generation_started_cb, NULL);
        evas_object_smart_callback_add(thumb, "generate,stop", _generation_finished_cb, NULL);
        evas_object_smart_callback_add(thumb, "generate,error", _generation_error_cb, NULL);
        
        elm_thumb_size_set(thumb, 200, 200);
        elm_thumb_editable_set(thumb, EINA_FALSE);
        snprintf(buf, sizeof(buf), "%s/%s", folder, lsdir->d_name );
        printf("**%s\n", buf);
        elm_thumb_file_set(thumb, buf, NULL);
        elm_thumb_reload(thumb);
            
        evas_object_size_hint_weight_set(thumb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_win_resize_object_add(th_win, thumb);
        elm_box_pack_end(th_box, thumb);
        evas_object_show(th_box);
        evas_object_show(thumb);
        elm_box_recalculate(th_box);
     }; 
   
     closedir(dir);
   }
}

static void
   _file_chosen(void *data, Evas_Object *obj, void *selected_item)
  {  
   Evas_Object *win, *image;
    
   char *file = selected_item;  
   int w, h, hg;

   if (file != NULL)
     {   
        list(file);
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


EAPI_MAIN int

elm_main(int argc, char **argv)
{
   Evas_Object *win, *box, *ic, *fs_bt, *en, *b_exit ;
   
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);  

/***********************************************************************************/   
        //elm_box_padding_set(th_box, 1, 1);
    //elm_box_homogeneous_set(th_box,EINA_TRUE);   
/***********************************************************************************/    
    win = elm_win_util_standard_add("reader", "Reader");
    evas_object_smart_callback_add(win, "delete,request", exit_cb, NULL);
    elm_win_autodel_set(win, EINA_TRUE);
    evas_object_resize(win, 600, 800);
    evas_object_show(win);
   
    box = elm_box_add(win);
    elm_box_horizontal_set(box, EINA_TRUE);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); 
    elm_win_resize_object_add(win, box);
    elm_box_padding_set(box, 1, 1);
    elm_box_homogeneous_set(box,EINA_TRUE);
    evas_object_show(box);
   
    ic = elm_icon_add(win);
    elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_THEME_FDO);
    elm_icon_standard_set(ic, "menu/folder");
    evas_object_show(ic);
  
    fs_bt = elm_fileselector_button_add(win);
    elm_fileselector_button_path_set(fs_bt, "/home");
    elm_object_text_set(fs_bt, "Select a file");
    elm_object_part_content_set(fs_bt, "icon", ic);
    elm_box_pack_end(box, fs_bt);
    elm_fileselector_button_inwin_mode_set(fs_bt, EINA_TRUE);
    evas_object_show(fs_bt);
 
    en = elm_entry_add(win);
    elm_entry_line_wrap_set(en, EINA_FALSE);
    elm_entry_editable_set(en, EINA_FALSE);
    evas_object_smart_callback_add(fs_bt, "file,chosen", _file_chosen, en);
    elm_box_pack_end(box, en);
    evas_object_show(en);

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

