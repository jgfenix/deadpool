#ifndef ELM_WIDGET_ENTRY_H
#define ELM_WIDGET_ENTRY_H

#include "elm_interface_scrollable.h"
#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-entry-class The Elementary Entry Class
 *
 * Elementary, besides having the @ref Entry widget, exposes its
 * foundation -- the Elementary Entry Class -- in order to create
 * other widgets which are a entry with some more logic on top.
 */

typedef struct _Mod_Api                     Mod_Api;

/**
 * Base widget smart data extended with entry instance data.
 */
typedef struct _Elm_Entry_Smart_Data        Elm_Entry_Smart_Data;
struct _Elm_Entry_Smart_Data
{
   Evas_Object                          *hit_rect, *entry_edje, *scr_edje;

   Evas_Object                          *hoversel;
   Evas_Object                          *mgf_bg;
   Evas_Object                          *mgf_clip;
   Evas_Object                          *mgf_proxy;
   Ecore_Job                            *deferred_recalc_job;
   Ecore_Event_Handler                  *sel_notify_handler;
   Ecore_Event_Handler                  *sel_clear_handler;
   Ecore_Timer                          *longpress_timer;
   Ecore_Timer                          *delay_write;
   /* for deferred appending */
   Ecore_Idler                          *append_text_idler;
   char                                 *append_text_left;
   int                                   append_text_position;
   int                                   append_text_len;
   /* Only for clipboard */
   const char                           *cut_sel;
   const char                           *text;
   const char                           *file;
   Elm_Text_Format                       format;
   Evas_Coord                            last_w, ent_mw, ent_mh;
   Evas_Coord                            downx, downy;
   Eina_List                            *items; /** context menu item list */
   Eina_List                            *item_providers;
   Eina_List                            *markup_filters;
   Ecore_Job                            *hov_deljob;
   Mod_Api                              *api; // module api if supplied
   int                                   cursor_pos;
   Elm_Scroller_Policy                   policy_h, policy_v;
   Elm_Wrap_Type                         line_wrap;
   Elm_Input_Panel_Layout                input_panel_layout;
   Elm_Autocapital_Type                  autocapital_type;
   Elm_Input_Panel_Lang                  input_panel_lang;
   Elm_Input_Panel_Return_Key_Type       input_panel_return_key_type;
   void                                 *input_panel_imdata;
   int                                   input_panel_imdata_len;
   int                                   input_panel_layout_variation;
   struct
     {
        Evas_Object *hover_parent; /**< hover parent object. entry is a hover parent object by default */
        Evas_Object *pop; /**< hidden icon for hover target */
        Evas_Object *hover; /**< hover object */
        const char  *hover_style; /**< style of a hover object */
     } anchor_hover;

   Elm_Cnp_Mode                          cnp_mode;

   Eina_Bool                             input_panel_return_key_disabled : 1;
   Eina_Bool                             drag_selection_asked : 1;
   Eina_Bool                             input_panel_enable : 1;
   Eina_Bool                             prediction_allow : 1;
   Eina_Bool                             selection_asked : 1;
   Eina_Bool                             auto_return_key : 1;
   Eina_Bool                             have_selection : 1;
   Eina_Bool                             deferred_cur : 1;
   Eina_Bool                             context_menu : 1;
   Eina_Bool                             long_pressed : 1;
   Eina_Bool                             cur_changed : 1;
   Eina_Bool                             single_line : 1;
   Eina_Bool                             can_write : 1;
   Eina_Bool                             auto_save : 1;
   Eina_Bool                             password : 1;
   Eina_Bool                             editable : 1;
   Eina_Bool                             disabled : 1;
   Eina_Bool                             h_bounce : 1;
   Eina_Bool                             v_bounce : 1;
   Eina_Bool                             has_text : 1;
   Eina_Bool                             use_down : 1;
   Eina_Bool                             sel_mode : 1;
   Eina_Bool                             changed : 1;
   Eina_Bool                             scroll : 1;
};

typedef struct _Elm_Entry_Item_Provider     Elm_Entry_Item_Provider;
typedef struct _Elm_Entry_Markup_Filter     Elm_Entry_Markup_Filter;

struct _Elm_Entry_Context_Menu_Item
{
   Evas_Object  *obj;
   const char   *label;
   const char   *icon_file;
   const char   *icon_group;
   Elm_Icon_Type icon_type;
   Evas_Smart_Cb func;
   void         *data;
};

struct _Elm_Entry_Item_Provider
{
   Evas_Object *(*func)(void *data, Evas_Object * entry, const char *item);
   void        *data;
};

struct _Elm_Entry_Markup_Filter
{
   Elm_Entry_Filter_Cb func;
   void               *data;
   void               *orig_data;
};

typedef enum _Length_Unit
{
   LENGTH_UNIT_CHAR,
   LENGTH_UNIT_BYTE,
   LENGTH_UNIT_LAST
} Length_Unit;

/**
 * @}
 */

#define ELM_ENTRY_DATA_GET(o, sd) \
  Elm_Entry_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_ENTRY_CLASS)

#define ELM_ENTRY_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_ENTRY_DATA_GET(o, ptr);                        \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_ENTRY_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_ENTRY_DATA_GET(o, ptr);                         \
  if (!ptr)                                           \
    {                                                 \
       CRITICAL("No widget data for object %p (%s)",  \
                o, evas_object_type_get(o));          \
       return val;                                    \
    }

#define ELM_ENTRY_CHECK(obj)                     \
  if (!eo_isa((obj), ELM_OBJ_ENTRY_CLASS))       \
    return

#endif
