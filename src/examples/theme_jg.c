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

   else  // strcmp(part, "elm.text.sub.2")
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
}

static void _part_info(void *data, Evas_Object *o, void *event_info)
{
   Evas_Object *list = data;
   Elm_Object_Item *glit = elm_genlist_selected_item_get(list);

   printf("\nSelected item pointer = %p\n",glit);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win;
   Evas_Object *list;
      Evas_Object *box, *hbox, *btn;
   int i;

   elm_theme_overlay_add(NULL, "./theme_jg.edj");

   win = elm_win_util_standard_add("genlist", "Genlist_jg");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_autodel_set(win, EINA_TRUE);
  
   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);
   
      if (!_itc)
     {
        _itc = elm_genlist_item_class_new();
        _itc->item_style = "double_label";//jg_double_label
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
      //elm_win_resize_object_add(win, list);  
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);

   elm_box_pack_end(box, list);
   evas_object_show(list);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(hbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, hbox);
   evas_object_show(hbox);
  

   
   btn = elm_button_add(win);
   elm_object_text_set(btn, "part information");
   evas_object_size_hint_weight_set(btn, 0, 0);
   evas_object_size_hint_align_set(btn, 0.5, 0.5);
   evas_object_smart_callback_add(btn, "clicked", _part_info, list);
   elm_box_pack_end(hbox, btn);
   evas_object_show(btn);

   
   evas_object_show(btn);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()

