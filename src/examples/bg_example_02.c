//Compile with:
//gcc -o bg_example_02 bg_example_02.c -g `pkg-config --cflags --libs elementary`
//where directory is the a path where images/plant_01.jpg can be found.

#include <Elementary.h>

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *box, *bg, *layout, *bt;
   char buf[PATH_MAX];

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_app_info_set(elm_main, "elementary", "images/cube1.png");// I created a directory in /usr/local/share/elementary/images and changed the image
      
   win = elm_win_util_standard_add("win", "Win"); //elm_win_add(NULL, "bg-image",ELM_WIN_BASIC);
   elm_win_title_set(win, "Bg Image");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);
   
   box = elm_box_add(win);
   elm_box_horizontal_set(box, EINA_TRUE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); 
   elm_win_resize_object_add(win, box);
   evas_object_show(box);
   
  /* layout = elm_layout_add(win);
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, layout);
   elm_layout_text_set(layout, buf, "TESTE");
   evas_object_show(layout);*/
 
   bg = elm_bg_add(win);

   elm_bg_load_size_set(bg, 20, 20);
   //elm_bg_option_set(bg, ELM_BG_OPTION_STRETCH);//see elm_bg_common.h
   snprintf(buf, sizeof(buf), "%s/images/cube1.png", elm_app_data_dir_get());
   elm_bg_file_set(bg, buf, NULL);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);// see elm_win_legacy.h 
   evas_object_show(bg); 

   bt = elm_button_add(win);
   elm_object_text_set(bt, " click ");
   elm_box_pack_start(box, bt);
   evas_object_size_hint_min_set(bt, 60, 40); 
   evas_object_show(bt);

   //evas_object_show(layout);
    
   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()


