#include<Elementary.h>

#ifndef PACKAGE_DATA_DIR
#define PACKAGE_DATA_DIR "."
#endif

#define WIDTH 300
#define HEIGHT 300

static Evas_Object *create_my_group(Evas *canvas, const char *text)
{
   Evas_Object *edje;
   edje = edje_object_add(canvas);
   if (!edje)
     {
        EINA_LOG_CRIT("could not create edje object!");
        return NULL;
     }
	if (!edje_object_file_set(edje, "theme_jg.edj", "my_group"))
    {
       int err = edje_object_load_error_get(edje);
       const char *errmsg = edje_load_error_str(err);
      fprintf(stderr, "could not load 'group_name' from theme.edj: %s",
       	errmsg);
 
       evas_object_del(edje);
       return NULL;
    }

   if (text)

        if (!edje_object_part_text_set(edje, "text", text))
          {
             EINA_LOG_WARN("could not set the text. "
                           "Maybe part 'text' does not exist?");
          }

   evas_object_move(edje, 0, 0);
   evas_object_resize(edje, WIDTH, HEIGHT);
   evas_object_show(edje);
   return edje;
}

EAPI_MAIN int
elm_main(int argc, char *argv[])
{
   Ecore_Evas *window;
   Evas *canvas;
   Evas_Object *edje;//, *bg, *win;
   const char *text;
  // char buf[PATH_MAX];
  
   ecore_evas_init();
   edje_init();

   window = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!window)
     {
        EINA_LOG_CRIT("could not create window.");
        return -1;
     }
   canvas = ecore_evas_get(window);

   text = (argc > 1) ? argv[1] : NULL;

	
   edje = create_my_group(canvas, text);
   if (!edje)
     return -2;

   ecore_evas_show(window);
  
 /*  elm_app_info_set(elm_main, "elementary", "images/cube1.png");
      
   win = elm_win_add(NULL, "bg-image", ELM_WIN_BASIC);
   elm_win_title_set(win, "Bg Image");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, WIDTH, HEIGHT);
   evas_object_show(win);
   
   bg = elm_bg_add(win);
   elm_bg_load_size_set(bg, 20, 20);
   snprintf(buf, sizeof(buf), "%s/images/cube1.png", elm_app_data_dir_get());
   elm_bg_file_set(bg, buf, NULL);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);      

   elm_run();*/

   ecore_main_loop_begin();
   
   evas_object_del(edje);
   ecore_evas_free(window);

   //elm_shutdown();
   edje_shutdown();
   ecore_evas_shutdown();
 
   return 0;
}
ELM_MAIN()
