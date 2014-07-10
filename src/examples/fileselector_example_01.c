/*
 * Elementary's file selector example to show thumbnails and images.
 *
 * If you select an image file(jpg, jpeg, png), a thumbnail will be displayed on the right
 * side of the winM, if you choose an image file, this widget will show it in an independet window,
 * if you choose a folder, the widget will scan the folder and create a roll of thumbnails buttons with
 * the files, if it's not a image file, a generic thumbnail will be displayed
 *
*/ 

#include <Elementary.h>
Evas_Object *panes, *winM, *win;

static const char generic_thumb[] = "/usr/share/elementary/images/icon_06.png";

static char *bt_hold_thumb;//hold the file when you select an image

static void
   _file_chosen(void *data, Evas_Object *obj, void *selected_item);

static void
  exit_cb(void *data, Evas_Object *obj, void *event_info)
  {
          elm_exit();
  }

 static void //exit thumbnail window
  exit_th_win(void *data, Evas_Object *obj, void *event_info)
  {
          evas_object_del(event_info);
  }

/*   Callback to list a path from file_chosen
 *   If it's a path, it will test if the files inside the path are 
 *   images and create thumbnails buttons in window th_win */
static void 
 list(char *folder)
{
  Evas_Object *th_win, *th_box, *thumb, *image, *bt;

  th_win = elm_win_util_standard_add("image_file", "thumbs");
  evas_object_smart_callback_add(th_win, "delete,request", exit_th_win, NULL);
  elm_win_autodel_set(th_win, EINA_TRUE); 
  evas_object_resize(th_win, 1, 1);
  
  th_box = elm_box_add(th_win);
  elm_box_horizontal_set(th_box, EINA_TRUE);
  evas_object_size_hint_weight_set(th_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); 
  elm_win_resize_object_add(th_win, th_box);
  elm_box_padding_set(th_box, 5, 5);

  image = elm_image_add(th_win);

  char file[PATH_MAX];
  char buf[PATH_MAX];

  DIR *dir;
  struct dirent *lsdir; 

  elm_need_ethumb();
        
   if(opendir(folder) != NULL)
   {
     evas_object_show(th_win);   
     evas_object_show(th_box);
     
     dir = opendir(folder);
     while ( ( lsdir = readdir(dir) ) != NULL )
      {
         if(strcmp(lsdir->d_name, ".") == 0 || strcmp(lsdir->d_name, "..") == 0  )
          continue;
        
        snprintf(file, sizeof(file), "%s", lsdir->d_name );
        printf("NAME OF FILE == %s\n",file);

        snprintf(buf, sizeof(buf), "%s/%s", folder, lsdir->d_name );

        thumb = elm_thumb_add(th_win);
     
/* this if will test if the path in 'buf' is an image, if not, it will change 
   the path to generic_thumb*/        
        if (!elm_image_file_set(image, buf, NULL))
            snprintf(buf, sizeof(generic_thumb), "%s",generic_thumb);
        printf("PATH == %s\n\n", buf);
              
        elm_thumb_editable_set(thumb, EINA_FALSE);
        elm_thumb_file_set(thumb, buf, NULL);
        elm_thumb_reload(thumb);
        
        bt = elm_button_add(th_win);
        elm_object_text_set(bt, file);
        elm_object_part_content_set(bt, "icon", thumb);
        elm_box_pack_end(th_box, bt);
        evas_object_size_hint_min_set(bt, 100, 100);
        evas_object_show(bt);

        elm_box_recalculate(th_box);  
      }; 
     closedir(dir);
   };
}

/*  Callback to show image chosen by button fs
 *  This callback receives a path from fs_bt and test if it's a image or folder.
 *  If it's a folder, list(*file) is called and will make a roll of thumbnails in 
 *  another independent window with the content of folder.
 *  If it's a valid image, it will show the image in another independent window   */
static void
   _file_chosen(void *data, Evas_Object *obj, void *selected_item)
 {  
   evas_object_del(win);//delete previous window and make another to show other images
   
   Evas_Object  *image;
   char *file = NULL;

   if(selected_item == NULL)
     {
        if (bt_hold_thumb != NULL)
         file = bt_hold_thumb;
     }
   else file = selected_item;
   int w, h, hg;
   
   win = elm_win_util_standard_add("image",file);
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 500, 500);
   
   if (file != NULL)
     {   
       image = elm_image_add(win);
          
       if (!elm_image_file_set(image, file, NULL))
        {
          
          if(opendir(file) != NULL)
            list(file);
          
          else
            printf("\nerror: could not load image \"%s\"\n", file);
        }
       else 
         {
            elm_image_object_size_get(image, &w, &h);
                
            hg = w;//used in elm_image_prescale_set to make the right scale and show without low pixels
            if(hg < h) hg = h;
        
            if( w != 0 && h != 0)//if the width and height are greater than 0, make a window to show the image
             {
                printf("\nImage path: %s", file); 
                printf("\nwidth x height = %d x %d\n", w, h);
           
                elm_image_no_scale_set(image, EINA_FALSE);
                elm_image_prescale_set(image, hg);  
                elm_image_resizable_set(image, EINA_TRUE, EINA_TRUE);
                elm_image_smooth_set(image, EINA_TRUE);
                elm_image_orient_set(image, ELM_IMAGE_ORIENT_NONE);
                elm_image_aspect_fixed_set(image, EINA_TRUE);
                elm_image_fill_outside_set(image, EINA_FALSE);
                elm_image_editable_set(image, EINA_TRUE);
                evas_object_size_hint_weight_set(image, EVAS_HINT_FILL, EVAS_HINT_FILL);
           
                elm_win_resize_object_add(win, image);
                evas_object_show(win);
                evas_object_show(image);
             }; 
         };
     }
   else
     printf("File selection canceled.\n");
 }

/*  Callback to show thumbnails on the right of window 
 *  This callback receives a path from fs_bt and make a thumbnail button
 *  that can be clicked to show the image */    
static void
  _selected_file(void *data, Evas_Object *obj, char *path)
 {
   bt_hold_thumb = path;

   Evas_Object *thumb, *bt, *image;
   thumb = elm_thumb_add(obj);
   printf("PATH == %s\n", path);

   image = elm_image_add(obj);
 
   if(!elm_image_file_set(image, path, NULL)) 
      elm_thumb_file_set(thumb, generic_thumb, NULL);
   else
     elm_thumb_file_set(thumb,path, NULL);
   
   elm_thumb_reload(thumb);
   
   bt = elm_button_add(obj);
   elm_object_part_content_set(bt, "icon", thumb);
   evas_object_smart_callback_add(bt, "clicked", _file_chosen , NULL);
   evas_object_show(bt);
   
   elm_object_part_content_set(panes, "right", bt);
 }   


EAPI_MAIN int

elm_main(int argc, char **argv)
{
   Evas_Object *thumb, *ic, *fs, *en = NULL ;
   
    elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);  
  
    elm_need_ethumb();

   /*winM it's a 'master' win that holds everything*/
   
    winM = elm_win_util_standard_add("image/thumb", "Image_&_Thumb_Generator");
    evas_object_smart_callback_add(winM, "delete,request", exit_cb, NULL);
    elm_win_autodel_set(winM, EINA_TRUE);
    evas_object_resize(winM, 500, 600);
    evas_object_show(winM);
    
    panes = elm_panes_add(winM);
    evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(winM, panes);
    elm_panes_horizontal_set(panes, EINA_FALSE);
    evas_object_show(panes);

    thumb = elm_thumb_add(winM);
    elm_thumb_reload(thumb);
    
    ic = elm_icon_add(winM);
    elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_THEME_FDO);
    elm_icon_standard_set(ic, "menu/folder");
    evas_object_show(ic);
  
    fs = elm_fileselector_add(winM);
    elm_fileselector_expandable_set(fs, EINA_TRUE);
    elm_fileselector_path_set(fs, "/home");
    elm_object_text_set(fs, "Select a file");
    evas_object_size_hint_weight_set(fs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(fs, EVAS_HINT_FILL, EVAS_HINT_FILL);

    evas_object_smart_callback_add(fs, "selected", _selected_file, NULL);
    evas_object_smart_callback_add(fs, "done", _file_chosen, NULL);
    evas_object_show(fs);
    
    en = elm_entry_add(winM);
    elm_entry_line_wrap_set(en, EINA_FALSE);
    elm_entry_editable_set(en, EINA_FALSE);
    evas_object_smart_callback_add(fs, "file,chosen", _file_chosen, en);
    evas_object_show(en);

    elm_object_part_content_set(panes, "left", fs);
   
    elm_run();
   elm_shutdown();
   return 0;
}
 ELM_MAIN()


