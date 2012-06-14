#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_layout.h"

static const char INWIN_SMART_NAME[] = "elm_inwin";

#define ELM_INWIN_DATA_GET(o, sd) \
  Elm_Layout_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_INWIN_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_INWIN_DATA_GET(o, ptr);                        \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_INWIN_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_INWIN_DATA_GET(o, ptr);                         \
  if (!ptr)                                           \
    {                                                 \
       CRITICAL("No widget data for object %p (%s)",  \
                o, evas_object_type_get(o));          \
       return val;                                    \
    }

#define ELM_INWIN_CHECK(obj)                                             \
  if (!obj || !elm_widget_type_check((obj), INWIN_SMART_NAME, __func__)) \
    return

/* Inheriting from elm_layout. Besides, we need no more than what is
 * there */
EVAS_SMART_SUBCLASS_NEW
  (INWIN_SMART_NAME, _elm_inwin, Elm_Layout_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, NULL);

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"default", "elm.swallow.content"},
   {NULL, NULL}
};

static void
_elm_inwin_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Object *content;
   Evas_Coord minw = -1, minh = -1;

   ELM_INWIN_DATA_GET(obj, sd);

   content = elm_layout_content_get(obj, NULL);

   if (!content) return;

   evas_object_size_hint_min_get(content, &minw, &minh);
   edje_object_size_min_calc(ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh);

   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static Eina_Bool
_elm_inwin_smart_focus_next(const Evas_Object *obj,
                            Elm_Focus_Direction dir,
                            Evas_Object **next)
{
   Evas_Object *content;

   content = elm_layout_content_get(obj, NULL);

   /* attempt to follow focus cycle into sub-object */
   if (content)
     {
        elm_widget_focus_next_get(content, dir, next);
        if (*next)
          return EINA_TRUE;
     }

   *next = (Evas_Object *)obj;
   return EINA_FALSE;
}

static void
_elm_inwin_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Layout_Smart_Data);

   ELM_WIDGET_CLASS(_elm_inwin_parent_sc)->base.add(obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_highlight_ignore_set(obj, EINA_TRUE);
}

static void
_elm_inwin_smart_set_user(Elm_Layout_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_inwin_smart_add;

   ELM_WIDGET_CLASS(sc)->focus_next = _elm_inwin_smart_focus_next;

   sc->sizing_eval = _elm_inwin_smart_sizing_eval;

   sc->content_aliases = _content_aliases;
}

EAPI Evas_Object *
elm_win_inwin_add(Evas_Object *parent)
{
   Evas *e;
   Evas_Object *obj;

   if (!parent || !elm_widget_type_check((parent), "elm_win", __func__))
     return NULL;  /* *has* to have a parent window */

   e = evas_object_evas_get(parent);
   if (!e) return NULL;

   obj = evas_object_smart_add(e, _elm_inwin_smart_class_new());

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(parent, obj);

   elm_layout_theme_set(obj, "win", "inwin", elm_object_style_get(obj));

   elm_layout_sizing_eval(obj);

   return obj;
}

EAPI void
elm_win_inwin_activate(Evas_Object *obj)
{
   ELM_INWIN_CHECK(obj);
   ELM_INWIN_DATA_GET_OR_RETURN(obj, sd);

   evas_object_raise(obj);
   evas_object_show(obj);
   edje_object_signal_emit
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm,action,show", "elm");
   elm_object_focus_set(obj, EINA_TRUE);
}

EAPI void
elm_win_inwin_content_set(Evas_Object *obj,
                          Evas_Object *content)
{
   ELM_INWIN_CHECK(obj);
   ELM_INWIN_DATA_GET_OR_RETURN(obj, sd);

   ELM_CONTAINER_CLASS(_elm_inwin_parent_sc)->content_set(obj, NULL, content);
}

EAPI Evas_Object *
elm_win_inwin_content_get(const Evas_Object *obj)
{
   ELM_INWIN_CHECK(obj) NULL;
   ELM_INWIN_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   return ELM_CONTAINER_CLASS(_elm_inwin_parent_sc)->content_get(obj, NULL);
}

EAPI Evas_Object *
elm_win_inwin_content_unset(Evas_Object *obj)
{
   ELM_INWIN_CHECK(obj) NULL;
   ELM_INWIN_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   return ELM_CONTAINER_CLASS(_elm_inwin_parent_sc)->content_unset(obj, NULL);
}