//Compile with:
//gcc -o bg_example_02 bg_example_02.c -g `pkg-config --cflags --libs elementary`
//where directory is the a path where images/plant_01.jpg can be found.

#include <Elementary.h>

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg ;
   char buf[PATH_MAX];

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_app_info_set(elm_main, "elementary", "images/cube1.png");// I created a directory in /usr/local/share/elementary/images and changed the image
      
   win = elm_win_add(NULL, "bg-image", ELM_WIN_BASIC);
   elm_win_title_set(win, "Bg Image");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);
   
   bg = elm_bg_add(win);
   elm_bg_load_size_set(bg, 20, 20);
   //elm_bg_option_set(bg, ELM_BG_OPTION_STRETCH);//see elm_bg_common.h
   snprintf(buf, sizeof(buf), "%s/images/cube1.png", elm_app_data_dir_get());
   elm_bg_file_set(bg, buf, NULL);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);// see elm_win_legacy.h 
   evas_object_show(bg);      

    
   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()


