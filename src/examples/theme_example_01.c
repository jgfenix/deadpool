/*
 * gcc -o theme_example_01 theme_example_01.c `pkg-config --cflags --libs elementary`
 */
#include <Elementary.h>
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

static void
btn_extension_click_cb(void *data, Evas_Object *btn, void *ev)
{
   const char *lbl = elm_object_text_get(btn);

   if (!strncmp(lbl, "Load", 4))
     {
        elm_theme_extension_add(NULL, "./theme_example.edj");
        elm_object_text_set(btn, "Unload extension");
     }
   else if (!strncmp(lbl, "Unload", 6))
     {
        elm_theme_extension_del(NULL, "./theme_example.edj");
        elm_object_text_set(btn, "Load extension");
     }
}

static void
btn_style_click_cb(void *data, Evas_Object *btn, void *ev)
{
   const char *styles[] = {
        "chucknorris",
        "default",
        "anchor"
   };
   static int sel_style = 0;

   sel_style = (sel_style + 1) % 3;
   elm_object_style_set(btn, styles[sel_style]);
}

EAPI_MAIN int
elm_main(int argc, char *argv[])
{

printf("\nARGC = %d\n",argc);
printf("\nARGV[1] = %s\n",argv[1]);
   Ecore_Evas *window;
   Evas *canvas;
   Evas_Object *edje;
   const char *text;

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


  // evas_object_del(edje);
  // ecore_evas_free(window);

   Evas_Object *win, *box, *btn;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   elm_theme_extension_add(NULL, "./theme_example.edj");
   elm_theme_overlay_add(NULL, "./theme_example.edj");
  
    box = elm_box_add(edje);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   btn = elm_button_add(edje);
   elm_object_text_set(btn, "Unload extension");
   elm_box_pack_end(box, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", btn_extension_click_cb, NULL);

   btn = elm_button_add(edje);
   elm_object_text_set(btn, "Switch style");
   elm_object_style_set(btn, "chucknorris");
   elm_box_pack_end(box, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", btn_style_click_cb, NULL);

   ecore_evas_show(window);
   ecore_main_loop_begin();

   elm_run();
   elm_shutdown();

   edje_shutdown();
   ecore_evas_shutdown();


   return 0;
}
ELM_MAIN()
