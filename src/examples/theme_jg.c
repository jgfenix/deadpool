//Compile with:
//gcc -g genlist_example_01.c -o genlist_example_01 `pkg-config --cflags --libs elementary`

#include <Elementary.h>

#define N_ITEMS 30

static Elm_Genlist_Item_Class *_itc = NULL;

static char *
_item_label_get(void *data, Evas_Object *obj, const char *part)
{
   char buf[256];
   int i = (int)(long)data;
   time_t t = (time_t)ecore_time_unix_get();
   

   if (!strcmp(part, "elm.text")) {
        snprintf(buf, sizeof(buf), "%s # %i", part,(int)(long)data);
   }
   
   else if (!strcmp(part, "elm.text.sub")) {
     snprintf(buf, sizeof(buf), "created at %s", ctime(&t));
     buf[strlen(buf) - 1] = '\0';

   }

   else// (!strcmp(part, "elm.text.sub.2"))
     snprintf(buf, sizeof(buf), "size ### %i", i);

   return strdup(buf);
}

static Evas_Object *
_item_content_get(void *data, Evas_Object *obj, const char *part)
{
   Evas_Object *ic = elm_icon_add(obj);

   if (!strcmp(part, "elm.swallow.icon"))
     elm_icon_standard_set(ic, "clock");

   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   return ic;
}

static void
_item_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
   printf("sel item data [%p] on genlist obj [%p], item pointer [%p]\n",
          data, obj, event_info);
    printf("** %s \n ",evas_object_name_get(data));
   /*if (!strcmp(part, "elm.text")) {
        snprintf(buf, sizeof(buf), "%s # %i", part,(int)(long)data);
   }*/


}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win;
   //Evas_Object *layout;
   Evas_Object *list;
   int i;
   //char my_dir[] = "/home/gabriel/src/elementary/src/examples/theme_jg.edj" ;

  // elm_theme_overlay_add(NULL, "./theme_jg.edj");

   win = elm_win_util_standard_add("genlist", "Genlist_jg");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_autodel_set(win, EINA_TRUE);

   /*layout = elm_layout_add(win);
   elm_layout_file_set(layout, my_dir, "elm/genlist/item/double_label/default");
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, layout);
   evas_object_show(layout);*/

   if (!_itc)
     {
        _itc = elm_genlist_item_class_new();
        _itc->item_style = "jg_double_label";
        _itc->func.text_get = _item_label_get;
        _itc->func.content_get = _item_content_get;
        _itc->func.state_get = NULL;
        _itc->func.del = NULL;
     }

   list = elm_genlist_add(win);

   for (i = 0; i < N_ITEMS; i++)
     {
        elm_genlist_item_append(list, _itc,
                                (void *)(long)i, NULL,
                                ELM_GENLIST_ITEM_NONE,
                                _item_sel_cb, NULL);
     }

   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, list);
   evas_object_show(list);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()

