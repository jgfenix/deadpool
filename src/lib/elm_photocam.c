#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_photocam.h"
#include "elm_interface_scrollable.h"

EAPI Eo_Op ELM_OBJ_PHOTOCAM_PAN_BASE_ID = EO_NOOP;

#define MY_PAN_CLASS ELM_OBJ_PHOTOCAM_PAN_CLASS

#define MY_PAN_CLASS_NAME "elm_photocam_pan"

EAPI Eo_Op ELM_OBJ_PHOTOCAM_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_PHOTOCAM_CLASS

#define MY_CLASS_NAME "elm_photocam"

/*
 * TODO (maybe - optional future stuff):
 *
 * 1. wrap photo in theme edje so u can have styling around photo (like white
 *    photo bordering).
 * 2. exif handling
 * 3. rotation flags in exif handling (nasty! should have rot in evas)
 */

static const char SIG_CLICKED[] = "clicked";
static const char SIG_PRESS[] = "press";
static const char SIG_LONGPRESSED[] = "longpressed";
static const char SIG_CLICKED_DOUBLE[] = "clicked,double";
static const char SIG_LOAD[] = "load";
static const char SIG_LOADED[] = "loaded";
static const char SIG_LOAD_DETAIL[] = "load,detail";
static const char SIG_LOADED_DETAIL[] = "loaded,detail";
static const char SIG_ZOOM_START[] = "zoom,start";
static const char SIG_ZOOM_STOP[] = "zoom,stop";
static const char SIG_ZOOM_CHANGE[] = "zoom,change";
static const char SIG_SCROLL[] = "scroll";
static const char SIG_SCROLL_ANIM_START[] = "scroll,anim,start";
static const char SIG_SCROLL_ANIM_STOP[] = "scroll,anim,stop";
static const char SIG_SCROLL_DRAG_START[] = "scroll,drag,start";
static const char SIG_SCROLL_DRAG_STOP[] = "scroll,drag,stop";
static const char SIG_DOWNLOAD_START[] = "download,start";
static const char SIG_DOWNLOAD_PROGRESS[] = "download,progress";
static const char SIG_DOWNLOAD_DONE[] = "download,done";
static const char SIG_DOWNLOAD_ERROR[] = "download,error";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CLICKED, ""},
   {SIG_PRESS, ""},
   {SIG_LONGPRESSED, ""},
   {SIG_CLICKED_DOUBLE, ""},
   {SIG_LOAD, ""},
   {SIG_LOADED, ""},
   {SIG_LOAD_DETAIL, ""},
   {SIG_LOADED_DETAIL, ""},
   {SIG_ZOOM_START, ""},
   {SIG_ZOOM_STOP, ""},
   {SIG_ZOOM_CHANGE, ""},
   {SIG_SCROLL, ""},
   {SIG_SCROLL_ANIM_START, ""},
   {SIG_SCROLL_ANIM_STOP, ""},
   {SIG_SCROLL_DRAG_START, ""},
   {SIG_SCROLL_DRAG_STOP, ""},
   {SIG_DOWNLOAD_START, ""},
   {SIG_DOWNLOAD_PROGRESS, ""},
   {SIG_DOWNLOAD_DONE, ""},
   {SIG_DOWNLOAD_ERROR, ""},
   {"focused", ""}, /**< handled by elm_widget */
   {"unfocused", ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

static inline void
_photocam_image_file_set(Evas_Object *obj, Elm_Photocam_Smart_Data *sd)
{
   if (sd->f)
     evas_object_image_mmap_set(obj, sd->f, NULL);
   else
     evas_object_image_file_set(obj, sd->file, NULL);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);

   evas_object_size_hint_max_get
     (wd->resize_obj, &maxw, &maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_calc_job_cb(void *data)
{
   Evas_Object *obj = data;
   ELM_PHOTOCAM_DATA_GET(obj, sd);
   Evas_Coord minw, minh;

   minw = sd->size.w;
   minh = sd->size.h;
   if (sd->resized)
     {
        sd->resized = EINA_FALSE;
        if (sd->mode != ELM_PHOTOCAM_ZOOM_MODE_MANUAL)
          {
             double tz = sd->zoom;
             sd->zoom = 0.0;
             elm_photocam_zoom_set(obj, tz);
          }
     }
   if ((minw != sd->minw) || (minh != sd->minh))
     {
        sd->minw = minw;
        sd->minh = minh;

        evas_object_smart_callback_call(sd->pan_obj, "changed", NULL);
        _sizing_eval(obj);
     }
   sd->calc_job = NULL;
   evas_object_smart_changed(sd->pan_obj);
}

static void
_elm_photocam_pan_smart_move(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Photocam_Pan_Smart_Data *psd = _pd;
   va_arg(*list, Evas_Coord);
   va_arg(*list, Evas_Coord);

   if (psd->wsd->calc_job) ecore_job_del(psd->wsd->calc_job);
   psd->wsd->calc_job = ecore_job_add(_calc_job_cb, psd->wobj);
}

static void
_elm_photocam_pan_smart_resize(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);
   Evas_Coord ow, oh;

   Elm_Photocam_Pan_Smart_Data *psd = _pd;

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;

   psd->wsd->resized = EINA_TRUE;
   if (psd->wsd->calc_job) ecore_job_del(psd->wsd->calc_job);
   psd->wsd->calc_job = ecore_job_add(_calc_job_cb, psd->wobj);
}

static void
_image_place(Evas_Object *obj,
             Evas_Coord px,
             Evas_Coord py,
             Evas_Coord ox,
             Evas_Coord oy,
             Evas_Coord ow,
             Evas_Coord oh)
{
   Evas_Coord ax, ay, gw, gh;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   ax = 0;
   ay = 0;
   gw = sd->size.w;
   gh = sd->size.h;
   if (!sd->zoom_g_layer)
     {
        if (ow > gw) ax = (ow - gw) / 2;
        if (oh > gh) ay = (oh - gh) / 2;
     }
   evas_object_move(sd->img, ox + 0 - px + ax, oy + 0 - py + ay);
   evas_object_resize(sd->img, gw, gh);

   if (sd->show.show)
     {
        sd->show.show = EINA_FALSE;
        eo_do(obj, elm_scrollable_interface_content_region_show
              (sd->show.x, sd->show.y, sd->show.w, sd->show.h));
     }
}

static void
_grid_load(Evas_Object *obj,
           Elm_Phocam_Grid *g)
{
   int x, y;
   Evas_Coord ox, oy, ow, oh, cvx, cvy, cvw, cvh, gw, gh, tx, ty;

   ELM_PHOTOCAM_DATA_GET(obj, sd);
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);

   evas_object_geometry_get(sd->pan_obj, &ox, &oy, &ow, &oh);
   evas_output_viewport_get(evas_object_evas_get(obj), &cvx, &cvy, &cvw, &cvh);

   gw = sd->size.w;
   gh = sd->size.h;
   for (y = 0; y < g->gh; y++)
     {
        for (x = 0; x < g->gw; x++)
          {
             int tn, xx, yy, ww, hh;
             Eina_Bool visible = EINA_FALSE;

             tn = (y * g->gw) + x;
             xx = g->grid[tn].out.x;
             yy = g->grid[tn].out.y;
             ww = g->grid[tn].out.w;
             hh = g->grid[tn].out.h;
             if ((gw != g->w) && (g->w > 0))
               {
                  tx = xx;
                  xx = (gw * xx) / g->w;
                  ww = ((gw * (tx + ww)) / g->w) - xx;
               }
             if ((gh != g->h) && (g->h > 0))
               {
                  ty = yy;
                  yy = (gh * yy) / g->h;
                  hh = ((gh * (ty + hh)) / g->h) - yy;
               }
             if (ELM_RECTS_INTERSECT(xx - sd->pan_x + ox,
                                     yy - sd->pan_y + oy,
                                     ww, hh, cvx, cvy, cvw, cvh))
               visible = EINA_TRUE;
             if ((visible) && (!g->grid[tn].have) && (!g->grid[tn].want))
               {
                  g->grid[tn].want = 1;
                  evas_object_hide(g->grid[tn].img);
                  evas_object_image_file_set(g->grid[tn].img, NULL, NULL);
                  evas_object_image_load_scale_down_set
                    (g->grid[tn].img, g->zoom);
                  evas_object_image_load_region_set
                    (g->grid[tn].img, g->grid[tn].src.x, g->grid[tn].src.y,
                    g->grid[tn].src.w, g->grid[tn].src.h);
                  _photocam_image_file_set(g->grid[tn].img, sd);
                  evas_object_image_preload(g->grid[tn].img, 0);
                  sd->preload_num++;
                  if (sd->preload_num == 1)
                    {
                       edje_object_signal_emit
                         (wd->resize_obj,
                         "elm,state,busy,start", "elm");
                       evas_object_smart_callback_call
                         (obj, SIG_LOAD_DETAIL, NULL);
                    }
               }
             else if ((g->grid[tn].want) && (!visible))
               {
                  sd->preload_num--;
                  if (!sd->preload_num)
                    {
                       edje_object_signal_emit
                         (wd->resize_obj,
                         "elm,state,busy,stop", "elm");
                       evas_object_smart_callback_call
                         (obj, SIG_LOADED_DETAIL, NULL);
                    }
                  g->grid[tn].want = 0;
                  evas_object_hide(g->grid[tn].img);
                  evas_object_image_preload(g->grid[tn].img, 1);
                  evas_object_image_file_set(g->grid[tn].img, NULL, NULL);
               }
             else if ((g->grid[tn].have) && (!visible))
               {
                  g->grid[tn].have = 0;
                  evas_object_hide(g->grid[tn].img);
                  evas_object_image_preload(g->grid[tn].img, 1);
                  evas_object_image_file_set(g->grid[tn].img, NULL, NULL);
               }
          }
     }
}

static void
_grid_place(Evas_Object *obj,
            Elm_Phocam_Grid *g,
            Evas_Coord px,
            Evas_Coord py,
            Evas_Coord ox,
            Evas_Coord oy,
            Evas_Coord ow,
            Evas_Coord oh)
{
   Evas_Coord ax, ay, gw, gh, tx, ty;
   int x, y;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   ax = 0;
   ay = 0;
   gw = sd->size.w;
   gh = sd->size.h;
   if (!sd->zoom_g_layer)
     {
        if (ow > gw) ax = (ow - gw) / 2;
        if (oh > gh) ay = (oh - gh) / 2;
     }
   for (y = 0; y < g->gh; y++)
     {
        for (x = 0; x < g->gw; x++)
          {
             int tn, xx, yy, ww, hh;

             tn = (y * g->gw) + x;
             xx = g->grid[tn].out.x;
             yy = g->grid[tn].out.y;
             ww = g->grid[tn].out.w;
             hh = g->grid[tn].out.h;
             if ((gw != g->w) && (g->w > 0))
               {
                  tx = xx;
                  xx = (gw * xx) / g->w;
                  ww = ((gw * (tx + ww)) / g->w) - xx;
               }
             if ((gh != g->h) && (g->h > 0))
               {
                  ty = yy;
                  yy = (gh * yy) / g->h;
                  hh = ((gh * (ty + hh)) / g->h) - yy;
               }
             evas_object_move(g->grid[tn].img,
                              ox + xx - px + ax,
                              oy + yy - py + ay);
             evas_object_resize(g->grid[tn].img, ww, hh);
          }
     }
}

static void
_elm_photocam_pan_smart_calculate(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Phocam_Grid *g;
   Eina_List *l;
   Evas_Coord ox, oy, ow, oh;

   Elm_Photocam_Pan_Smart_Data *psd = _pd;
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(psd->wobj, ELM_OBJ_WIDGET_CLASS);

   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   _image_place(
       wd->obj, psd->wsd->pan_x, psd->wsd->pan_y,
       ox - psd->wsd->g_layer_zoom.imx, oy - psd->wsd->g_layer_zoom.imy, ow,
       oh);

   EINA_LIST_FOREACH(psd->wsd->grids, l, g)
     {
        _grid_load(wd->obj, g);
        _grid_place(
              wd->obj, g, psd->wsd->pan_x,
             psd->wsd->pan_y, ox - psd->wsd->g_layer_zoom.imx,
             oy - psd->wsd->g_layer_zoom.imy, ow, oh);
     }
}

static void
_elm_photocam_pan_smart_pos_set(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   Elm_Photocam_Pan_Smart_Data *psd = _pd;

   if ((x == psd->wsd->pan_x) && (y == psd->wsd->pan_y)) return;
   psd->wsd->pan_x = x;
   psd->wsd->pan_y = y;
   evas_object_smart_changed(obj);
}

static void
_elm_photocam_pan_smart_pos_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord *x = va_arg(*list, Evas_Coord *);
   Evas_Coord *y = va_arg(*list, Evas_Coord *);
   Elm_Photocam_Pan_Smart_Data *psd = _pd;

   if (x) *x = psd->wsd->pan_x;
   if (y) *y = psd->wsd->pan_y;
}

static void
_elm_photocam_pan_smart_pos_max_get(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord ow, oh;
   Evas_Coord *x = va_arg(*list, Evas_Coord *);
   Evas_Coord *y = va_arg(*list, Evas_Coord *);
   Elm_Photocam_Pan_Smart_Data *psd = _pd;

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   ow = psd->wsd->minw - ow;
   if (ow < 0) ow = 0;
   oh = psd->wsd->minh - oh;
   if (oh < 0) oh = 0;
   if (x) *x = ow;
   if (y) *y = oh;
}

static void
_elm_photocam_pan_smart_pos_min_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord *x = va_arg(*list, Evas_Coord *);
   Evas_Coord *y = va_arg(*list, Evas_Coord *);
   if (x) *x = 0;
   if (y) *y = 0;
}

static void
_elm_photocam_pan_smart_content_size_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord *w = va_arg(*list, Evas_Coord *);
   Evas_Coord *h = va_arg(*list, Evas_Coord *);
   Elm_Photocam_Pan_Smart_Data *psd = _pd;

   if (w) *w = psd->wsd->minw;
   if (h) *h = psd->wsd->minh;
}

static void
_elm_photocam_pan_destructor(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Photocam_Pan_Smart_Data *psd = _pd;
   eo_data_unref(psd->wobj, psd->wsd);
   eo_do_super(obj, MY_PAN_CLASS, eo_destructor());
}

static void
_photocam_pan_class_constructor(Eo_Class *klass)
{
      const Eo_Op_Func_Description func_desc[] = {
           EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _elm_photocam_pan_destructor),

           EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_RESIZE), _elm_photocam_pan_smart_resize),
           EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MOVE), _elm_photocam_pan_smart_move),
           EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALCULATE), _elm_photocam_pan_smart_calculate),
           EO_OP_FUNC(ELM_OBJ_PAN_ID(ELM_OBJ_PAN_SUB_ID_POS_SET), _elm_photocam_pan_smart_pos_set),
           EO_OP_FUNC(ELM_OBJ_PAN_ID(ELM_OBJ_PAN_SUB_ID_POS_GET), _elm_photocam_pan_smart_pos_get),
           EO_OP_FUNC(ELM_OBJ_PAN_ID(ELM_OBJ_PAN_SUB_ID_POS_MAX_GET), _elm_photocam_pan_smart_pos_max_get),
           EO_OP_FUNC(ELM_OBJ_PAN_ID(ELM_OBJ_PAN_SUB_ID_POS_MIN_GET), _elm_photocam_pan_smart_pos_min_get),
           EO_OP_FUNC(ELM_OBJ_PAN_ID(ELM_OBJ_PAN_SUB_ID_CONTENT_SIZE_GET), _elm_photocam_pan_smart_content_size_get),
           EO_OP_FUNC_SENTINEL
      };
      eo_class_funcs_set(klass, func_desc);

      evas_smart_legacy_type_register(MY_PAN_CLASS_NAME, klass);
}

static const Eo_Class_Description _elm_photocam_pan_class_desc = {
     EO_VERSION,
     MY_PAN_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     sizeof(Elm_Photocam_Pan_Smart_Data),
     _photocam_pan_class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_photocam_pan_class_get, &_elm_photocam_pan_class_desc, ELM_OBJ_PAN_CLASS, NULL);

static int
_nearest_pow2_get(int num)
{
   unsigned int n = num - 1;

   n |= n >> 1;
   n |= n >> 2;
   n |= n >> 4;
   n |= n >> 8;
   n |= n >> 16;

   return n + 1;
}

static void
_grid_clear(Evas_Object *obj,
            Elm_Phocam_Grid *g)
{
   int x, y;

   ELM_PHOTOCAM_DATA_GET(obj, sd);
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);

   if (!g->grid) return;
   for (y = 0; y < g->gh; y++)
     {
        for (x = 0; x < g->gw; x++)
          {
             int tn;

             tn = (y * g->gw) + x;
             evas_object_del(g->grid[tn].img);
             if (g->grid[tn].want)
               {
                  sd->preload_num--;
                  if (!sd->preload_num)
                    {
                       edje_object_signal_emit
                         (wd->resize_obj,
                         "elm,state,busy,stop", "elm");
                       evas_object_smart_callback_call
                         (obj, SIG_LOAD_DETAIL, NULL);
                    }
               }
          }
     }

   free(g->grid);
   g->grid = NULL;
   g->gw = 0;
   g->gh = 0;
}

static void
_tile_preloaded_cb(void *data,
                   Evas *e __UNUSED__,
                   Evas_Object *o __UNUSED__,
                   void *event_info __UNUSED__)
{
   Elm_Photocam_Grid_Item *git = data;
   ELM_PHOTOCAM_DATA_GET(git->obj, sd);
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(git->obj, ELM_OBJ_WIDGET_CLASS);

   if (git->want)
     {
        git->want = 0;
        evas_object_show(git->img);
        git->have = 1;
        sd->preload_num--;
        if (!sd->preload_num)
          {
             edje_object_signal_emit
               (wd->resize_obj, "elm,state,busy,stop",
               "elm");
             evas_object_smart_callback_call
               (wd->obj, SIG_LOADED_DETAIL, NULL);
          }
     }
}

static int
_grid_zoom_calc(double zoom)
{
   int z = zoom;

   if (z < 1) z = 1;
   return _nearest_pow2_get(z);
}

static Elm_Phocam_Grid *
_grid_create(Evas_Object *obj)
{
   int x, y;
   Elm_Phocam_Grid *g;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   g = calloc(1, sizeof(Elm_Phocam_Grid));
   if (!g) return NULL;

   g->zoom = _grid_zoom_calc(sd->zoom);
   g->tsize = sd->tsize;
   g->iw = sd->size.imw;
   g->ih = sd->size.imh;

   g->w = g->iw / g->zoom;
   g->h = g->ih / g->zoom;
   if (g->zoom >= 8)
     {
        free(g);

        return NULL;
     }
   if (sd->do_region)
     {
        g->gw = (g->w + g->tsize - 1) / g->tsize;
        g->gh = (g->h + g->tsize - 1) / g->tsize;
     }
   else
     {
        g->gw = 1;
        g->gh = 1;
     }

   g->grid = calloc(1, sizeof(Elm_Photocam_Grid_Item) * g->gw * g->gh);
   if (!g->grid)
     {
        g->gw = 0;
        g->gh = 0;

        return g;
     }

   for (y = 0; y < g->gh; y++)
     {
        for (x = 0; x < g->gw; x++)
          {
             int tn;

             tn = (y * g->gw) + x;
             g->grid[tn].src.x = x * g->tsize;
             if (x == (g->gw - 1))
               g->grid[tn].src.w = g->w - ((g->gw - 1) * g->tsize);
             else
               g->grid[tn].src.w = g->tsize;
             g->grid[tn].src.y = y * g->tsize;
             if (y == (g->gh - 1))
               g->grid[tn].src.h = g->h - ((g->gh - 1) * g->tsize);
             else
               g->grid[tn].src.h = g->tsize;

             g->grid[tn].out.x = g->grid[tn].src.x;
             g->grid[tn].out.y = g->grid[tn].src.y;
             g->grid[tn].out.w = g->grid[tn].src.w;
             g->grid[tn].out.h = g->grid[tn].src.h;

             g->grid[tn].obj = obj;
             g->grid[tn].img =
               evas_object_image_add(evas_object_evas_get(obj));
             evas_object_image_load_orientation_set(g->grid[tn].img, EINA_TRUE);
             evas_object_image_scale_hint_set
               (g->grid[tn].img, EVAS_IMAGE_SCALE_HINT_DYNAMIC);
             evas_object_pass_events_set(g->grid[tn].img, EINA_TRUE);

             /* XXX: check this */
             evas_object_smart_member_add(g->grid[tn].img, sd->pan_obj);
             elm_widget_sub_object_add(obj, g->grid[tn].img);
             evas_object_image_filled_set(g->grid[tn].img, 1);
             evas_object_event_callback_add
               (g->grid[tn].img, EVAS_CALLBACK_IMAGE_PRELOADED,
               _tile_preloaded_cb, &(g->grid[tn]));
          }
     }

   return g;
}

static void
_grid_clear_all(Evas_Object *obj)
{
   Elm_Phocam_Grid *g;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   EINA_LIST_FREE(sd->grids, g)
     {
        _grid_clear(obj, g);
        free(g);
     }
}

static void
_smooth_update(Evas_Object *obj)
{
   Elm_Phocam_Grid *g;
   int x, y;
   Eina_List *l;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   EINA_LIST_FOREACH(sd->grids, l, g)
     {
        for (y = 0; y < g->gh; y++)
          {
             for (x = 0; x < g->gw; x++)
               {
                  int tn;

                  tn = (y * g->gw) + x;
                  evas_object_image_smooth_scale_set
                    (g->grid[tn].img, (!sd->no_smooth));
               }
          }
     }

   evas_object_image_smooth_scale_set(sd->img, (!sd->no_smooth));
}

static void
_grid_raise(Elm_Phocam_Grid *g)
{
   int x, y;

   for (y = 0; y < g->gh; y++)
     {
        for (x = 0; x < g->gw; x++)
          {
             int tn;

             tn = (y * g->gw) + x;
             evas_object_raise(g->grid[tn].img);
          }
     }
}

static Eina_Bool
_scroll_timeout_cb(void *data)
{
   ELM_PHOTOCAM_DATA_GET(data, sd);

   sd->no_smooth--;
   if (!sd->no_smooth) _smooth_update(data);

   sd->scr_timer = NULL;

   return ECORE_CALLBACK_CANCEL;
}

static void
_main_img_preloaded_cb(void *data,
                       Evas *e __UNUSED__,
                       Evas_Object *o __UNUSED__,
                       void *event_info __UNUSED__)
{
   Evas_Object *obj = data;
   Elm_Phocam_Grid *g;

   ELM_PHOTOCAM_DATA_GET(data, sd);
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(data, ELM_OBJ_WIDGET_CLASS);

   evas_object_show(sd->img);
   sd->main_load_pending = 0;
   g = _grid_create(obj);
   if (g)
     {
        sd->grids = eina_list_prepend(sd->grids, g);
        _grid_load(obj, g);
     }
   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job_cb, data);
   evas_object_smart_callback_call(data, SIG_LOADED, NULL);
   sd->preload_num--;
   if (!sd->preload_num)
     {
        edje_object_signal_emit
          (wd->resize_obj, "elm,state,busy,stop", "elm");
        evas_object_smart_callback_call(obj, SIG_LOADED_DETAIL, NULL);
     }
}

static Eina_Bool
_zoom_do(Evas_Object *obj,
         double t)
{
   Evas_Coord xx, yy, ow, oh;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   sd->size.w = (sd->size.ow * (1.0 - t)) + (sd->size.nw * t);
   sd->size.h = (sd->size.oh * (1.0 - t)) + (sd->size.nh * t);
   eo_do(obj, elm_scrollable_interface_content_viewport_size_get(&ow, &oh));
   xx = (sd->size.spos.x * sd->size.w) - (ow / 2);
   yy = (sd->size.spos.y * sd->size.h) - (oh / 2);
   if (xx < 0) xx = 0;
   else if (xx > (sd->size.w - ow))
     xx = sd->size.w - ow;
   if (yy < 0) yy = 0;
   else if (yy > (sd->size.h - oh))
     yy = sd->size.h - oh;

   sd->show.show = EINA_TRUE;
   sd->show.x = xx;
   sd->show.y = yy;
   sd->show.w = ow;
   sd->show.h = oh;

   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job_cb, obj);
   if (t >= 1.0)
     {
        Eina_List *l, *l_next;
        Elm_Phocam_Grid *g;

        EINA_LIST_FOREACH_SAFE(sd->grids, l, l_next, g)
          {
             if (g->dead)
               {
                  sd->grids = eina_list_remove_list(sd->grids, l);
                  _grid_clear(obj, g);
                  free(g);
               }
          }
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_zoom_anim_cb(void *data)
{
   double t;
   Eina_Bool go;
   Evas_Object *obj = data;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   t = ecore_loop_time_get();
   if (t >= sd->t_end)
     t = 1.0;
   else if (sd->t_end > sd->t_start)
     t = (t - sd->t_start) / (sd->t_end - sd->t_start);
   else
     t = 1.0;
   t = 1.0 - t;
   t = 1.0 - (t * t);
   go = _zoom_do(obj, t);
   if (!go)
     {
        sd->no_smooth--;
        if (!sd->no_smooth) _smooth_update(data);
        sd->zoom_animator = NULL;
        evas_object_smart_callback_call(obj, SIG_ZOOM_STOP, NULL);
     }

   return go;
}

static Eina_Bool
_long_press_cb(void *data)
{
   ELM_PHOTOCAM_DATA_GET(data, sd);

   sd->long_timer = NULL;
   sd->longpressed = EINA_TRUE;
   evas_object_smart_callback_call(data, SIG_LONGPRESSED, NULL);

   return ECORE_CALLBACK_CANCEL;
}

static void
_mouse_down_cb(void *data,
               Evas *evas __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;

   ELM_PHOTOCAM_DATA_GET(data, sd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) sd->on_hold = EINA_TRUE;
   else sd->on_hold = EINA_FALSE;
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     evas_object_smart_callback_call(data, SIG_CLICKED_DOUBLE, NULL);
   else
     evas_object_smart_callback_call(data, SIG_PRESS, NULL);
   sd->longpressed = EINA_FALSE;
   if (sd->long_timer) ecore_timer_del(sd->long_timer);
   sd->long_timer = ecore_timer_add
       (_elm_config->longpress_timeout, _long_press_cb, data);
}

static void
_mouse_up_cb(void *data,
             Evas *evas __UNUSED__,
             Evas_Object *obj __UNUSED__,
             void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;

   ELM_PHOTOCAM_DATA_GET(data, sd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) sd->on_hold = EINA_TRUE;
   else sd->on_hold = EINA_FALSE;
   ELM_SAFE_FREE(sd->long_timer, ecore_timer_del);
   if (!sd->on_hold)
     evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
   sd->on_hold = EINA_FALSE;
}

static void
_elm_photocam_smart_on_focus(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_wdg_on_focus(&int_ret));
   if (!int_ret) return;

   if (elm_widget_focus_get(obj))
     {
        edje_object_signal_emit
          (wd->resize_obj, "elm,action,focus", "elm");
        evas_object_focus_set(wd->resize_obj, EINA_TRUE);
     }
   else
     {
        edje_object_signal_emit
          (wd->resize_obj, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->resize_obj, EINA_FALSE);
     }

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_photocam_smart_theme(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_wdg_theme(&int_ret));
   if (!int_ret) return;

   _sizing_eval(obj);

   if (ret) *ret = EINA_TRUE;
}

static void
_scroll_animate_start_cb(Evas_Object *obj,
                         void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_ANIM_START, NULL);
}

static void
_scroll_animate_stop_cb(Evas_Object *obj,
                        void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_ANIM_STOP, NULL);
}

static void
_scroll_drag_start_cb(Evas_Object *obj,
                      void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_DRAG_START, NULL);
}

static void
_scroll_drag_stop_cb(Evas_Object *obj,
                     void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_DRAG_STOP, NULL);
}

static void
_scroll_cb(Evas_Object *obj,
           void *data __UNUSED__)
{
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   if (!sd->scr_timer)
     {
        sd->no_smooth++;
        if (sd->no_smooth == 1) _smooth_update(obj);
     }

   if (sd->scr_timer) ecore_timer_del(sd->scr_timer);
   sd->scr_timer = ecore_timer_add(0.5, _scroll_timeout_cb, obj);

   evas_object_smart_callback_call(obj, SIG_SCROLL, NULL);
}

static void
_elm_photocam_smart_event(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *src = va_arg(*list, Evas_Object *);
   (void) src;
   Evas_Callback_Type type = va_arg(*list, Evas_Callback_Type);
   void *event_info = va_arg(*list, void *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   double zoom;
   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord v_w = 0;
   Evas_Coord v_h = 0;
   Evas_Coord step_x = 0;
   Evas_Coord step_y = 0;
   Evas_Coord page_x = 0;
   Evas_Coord page_y = 0;
   Evas_Event_Key_Down *ev = event_info;

   if (elm_widget_disabled_get(obj)) return;

   if (type != EVAS_CALLBACK_KEY_DOWN) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   eo_do(obj,
         elm_scrollable_interface_content_pos_get(&x, &y),
         elm_scrollable_interface_step_size_get(&step_x, &step_y),
         elm_scrollable_interface_page_size_get(&page_x, &page_y),
         elm_scrollable_interface_content_viewport_size_get(&v_w, &v_h));

   if ((!strcmp(ev->key, "Left")) ||
       ((!strcmp(ev->key, "KP_Left")) && (!ev->string)))
     {
        x -= step_x;
     }
   else if ((!strcmp(ev->key, "Right")) ||
            ((!strcmp(ev->key, "KP_Right")) && (!ev->string)))
     {
        x += step_x;
     }
   else if ((!strcmp(ev->key, "Up")) ||
            ((!strcmp(ev->key, "KP_Up")) && (!ev->string)))
     {
        y -= step_y;
     }
   else if ((!strcmp(ev->key, "Down")) ||
            ((!strcmp(ev->key, "KP_Down")) && (!ev->string)))
     {
        y += step_y;
     }
   else if ((!strcmp(ev->key, "Prior")) ||
            ((!strcmp(ev->key, "KP_Prior")) && (!ev->string)))
     {
        if (page_y < 0)
          y -= -(page_y * v_h) / 100;
        else
          y -= page_y;
     }
   else if ((!strcmp(ev->key, "Next")) ||
            ((!strcmp(ev->key, "KP_Next")) && (!ev->string)))
     {
        if (page_y < 0)
          y += -(page_y * v_h) / 100;
        else
          y += page_y;
     }
   else if ((!strcmp(ev->key, "KP_Add")))
     {
        zoom = elm_photocam_zoom_get(obj);
        zoom -= 0.5;
        elm_photocam_zoom_mode_set(obj, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
        elm_photocam_zoom_set(obj, zoom);
        if (ret) *ret = EINA_TRUE;
        return;
     }
   else if ((!strcmp(ev->key, "KP_Subtract")))
     {
        zoom = elm_photocam_zoom_get(obj);
        zoom += 0.5;
        elm_photocam_zoom_mode_set(obj, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
        elm_photocam_zoom_set(obj, zoom);
        if (ret) *ret = EINA_TRUE;
        return;
     }
   else return;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   eo_do(obj, elm_scrollable_interface_content_pos_set(x, y, EINA_TRUE));

   if (ret) *ret = EINA_TRUE;
}

Eina_Bool
_bounce_eval(void *data)
{
   Evas_Object *obj = data;
   ELM_PHOTOCAM_DATA_GET(obj, sd);
   double t, tt;

   if ((sd->g_layer_zoom.imx == sd->g_layer_zoom.bounce.x_end) &&
       (sd->g_layer_zoom.imy == sd->g_layer_zoom.bounce.y_end))
     {
        sd->g_layer_zoom.imx = 0;
        sd->g_layer_zoom.imy = 0;
        sd->zoom_g_layer = EINA_FALSE;
        sd->g_layer_zoom.bounce.animator = NULL;

        eo_do(obj, elm_scrollable_interface_freeze_set(EINA_FALSE));

        return ECORE_CALLBACK_CANCEL;
     }

   t = ecore_loop_time_get();
   tt = (t - sd->g_layer_zoom.bounce.t_start) /
     (sd->g_layer_zoom.bounce.t_end -
      sd->g_layer_zoom.bounce.t_start);
   tt = 1.0 - tt;
   tt = 1.0 - (tt * tt);

   if (t > sd->g_layer_zoom.bounce.t_end)
     {
        sd->g_layer_zoom.imx = 0;
        sd->g_layer_zoom.imy = 0;
        sd->zoom_g_layer = EINA_FALSE;

        eo_do(obj, elm_scrollable_interface_freeze_set(EINA_FALSE));

        _zoom_do(obj, 1.0);
        sd->g_layer_zoom.bounce.animator = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   if (sd->g_layer_zoom.imx != sd->g_layer_zoom.bounce.x_end)
     sd->g_layer_zoom.imx =
       sd->g_layer_zoom.bounce.x_start * (1.0 - tt) +
       sd->g_layer_zoom.bounce.x_end * tt;

   if (sd->g_layer_zoom.imy != sd->g_layer_zoom.bounce.y_end)
     sd->g_layer_zoom.imy =
       sd->g_layer_zoom.bounce.y_start * (1.0 - tt) +
       sd->g_layer_zoom.bounce.y_end * tt;

   _zoom_do(obj, 1.0 - (1.0 - tt));

   return ECORE_CALLBACK_RENEW;
}

static void
_g_layer_zoom_do(Evas_Object *obj,
                 Evas_Coord px,
                 Evas_Coord py,
                 Elm_Gesture_Zoom_Info *g_layer)
{
   int regx, regy, regw, regh, ix, iy, iw, ih;
   Evas_Coord rx, ry, rw, rh;
   int xx, yy;

   ELM_PHOTOCAM_DATA_GET(obj, sd);
   sd->mode = ELM_PHOTOCAM_ZOOM_MODE_MANUAL;
   sd->zoom = sd->g_layer_start / g_layer->zoom;
   sd->size.ow = sd->size.w;
   sd->size.oh = sd->size.h;
   eo_do(obj, elm_scrollable_interface_content_pos_get(&rx, &ry));
   eo_do(obj, elm_scrollable_interface_content_viewport_size_get(&rw, &rh));
   if ((rw <= 0) || (rh <= 0)) return;

   sd->size.nw = (double)sd->size.imw / sd->zoom;
   sd->size.nh = (double)sd->size.imh / sd->zoom;

   elm_photocam_image_region_get(obj, &regx, &regy, &regw, &regh);
   evas_object_geometry_get(sd->img, &ix, &iy, &iw, &ih);

   sd->pvx = g_layer->x;
   sd->pvy = g_layer->y;

   xx = (px / sd->zoom) - sd->pvx;
   yy = (py / sd->zoom) - sd->pvy;
   sd->g_layer_zoom.imx = 0;
   sd->g_layer_zoom.imy = 0;

   if ((xx < 0) || (rw > sd->size.nw))
     {
        sd->g_layer_zoom.imx = xx;
        xx = 0;
     }
   else if ((xx + rw) > sd->size.nw)
     {
        sd->g_layer_zoom.imx = xx + rw - sd->size.nw;
        xx = sd->size.nw - rw;
     }

   if ((yy < 0) || (rh > sd->size.nh))
     {
        sd->g_layer_zoom.imy = yy;
        yy = 0;
     }
   else if ((yy + rh) > sd->size.nh)
     {
        sd->g_layer_zoom.imy = yy + rh - sd->size.nh;
        yy = sd->size.nh - rh;
     }

   sd->size.spos.x = (double)(xx + (rw / 2)) / (double)(sd->size.nw);
   sd->size.spos.y = (double)(yy + (rh / 2)) / (double)(sd->size.nh);

   _zoom_do(obj, 1.0);
}

static Evas_Event_Flags
_g_layer_zoom_start_cb(void *data,
                       void *event_info)
{
   Evas_Object *obj = data;
   Elm_Gesture_Zoom_Info *p = event_info;
   ELM_PHOTOCAM_DATA_GET(obj, sd);
   double marginx = 0, marginy = 0;
   Evas_Coord rw, rh;
   int x, y, w, h;

   ELM_SAFE_FREE(sd->g_layer_zoom.bounce.animator, ecore_animator_del);
   sd->zoom_g_layer = EINA_TRUE;

   eo_do(obj, elm_scrollable_interface_freeze_set(EINA_TRUE));

   elm_photocam_image_region_get(obj, &x, &y, &w, &h);
   eo_do(obj, elm_scrollable_interface_content_viewport_size_get(&rw, &rh));

   if (rw > sd->size.nw)
     marginx = (rw - sd->size.nw) / 2;
   if (rh > sd->size.nh)
     marginy = (rh - sd->size.nh) / 2;

   sd->g_layer_start = sd->zoom;

   sd->zoom_point_x = x + ((p->x - marginx) * sd->zoom) +
     sd->g_layer_zoom.imx;
   sd->zoom_point_y = y + ((p->y - marginy) * sd->zoom) +
     sd->g_layer_zoom.imy;

   return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags
_g_layer_zoom_move_cb(void *data,
                      void *event_info)
{
   Elm_Photocam_Smart_Data *sd = eo_data_scope_get(data, MY_CLASS);
   Elm_Gesture_Zoom_Info *p = event_info;

   _g_layer_zoom_do(data, sd->zoom_point_x, sd->zoom_point_y, p);

   return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags
_g_layer_zoom_end_cb(void *data,
                     void *event_info __UNUSED__)
{
   Evas_Object *obj = data;
   ELM_PHOTOCAM_DATA_GET(obj, sd);
   Evas_Coord rw, rh;

   eo_do(obj, elm_scrollable_interface_content_viewport_size_get(&rw, &rh));
   sd->g_layer_start = 1.0;

   if (sd->g_layer_zoom.imx || sd->g_layer_zoom.imy)
     {
        double t;

        t = ecore_loop_time_get();
        sd->g_layer_zoom.bounce.x_start = sd->g_layer_zoom.imx;
        sd->g_layer_zoom.bounce.y_start = sd->g_layer_zoom.imy;
        sd->g_layer_zoom.bounce.x_end = 0;
        sd->g_layer_zoom.bounce.y_end = 0;

        if (rw > sd->size.nw &&
            rh > sd->size.nh)
          {
             Evas_Coord pw, ph;
             double z;

             if ((sd->size.imw < rw) && (sd->size.imh < rh))
               {
                  sd->zoom = 1;
                  sd->size.nw = sd->size.imw;
                  sd->size.nh = sd->size.imh;
               }
             else
               {
                  ph = (sd->size.imh * rw) / sd->size.imw;
                  if (ph > rh)
                    {
                       pw = (sd->size.imw * rh) / sd->size.imh;
                       ph = rh;
                    }
                  else
                    {
                       pw = rw;
                    }
                  if (sd->size.imw > sd->size.imh)
                    z = (double)sd->size.imw / pw;
                  else
                    z = (double)sd->size.imh / ph;

                  sd->zoom = z;
                  sd->size.nw = pw;
                  sd->size.nh = ph;
               }
             sd->g_layer_zoom.bounce.x_end = (sd->size.nw - rw) / 2;
             sd->g_layer_zoom.bounce.y_end = (sd->size.nh - rh) / 2;
          }
        else
          {
             int xx, yy;

             xx = (sd->zoom_point_x / sd->zoom) - sd->pvx;
             yy = (sd->zoom_point_y / sd->zoom) - sd->pvy;

             if (xx < 0) xx = 0;
             if (yy < 0) yy = 0;

             if (rw > sd->size.nw)
               sd->g_layer_zoom.bounce.x_end = (sd->size.nw - rw) / 2;
             if ((xx + rw) > sd->size.nw)
               xx = sd->size.nw - rw;

             if (rh > sd->size.nh)
               sd->g_layer_zoom.bounce.y_end = (sd->size.nh - rh) / 2;
             if ((yy + rh) > sd->size.nh)
               yy = sd->size.nh - rh;

             sd->size.spos.x = (double)(xx + (rw / 2)) / (double)(sd->size.nw);
             sd->size.spos.y = (double)(yy + (rh / 2)) / (double)(sd->size.nh);
          }

        sd->g_layer_zoom.bounce.t_start = t;
        sd->g_layer_zoom.bounce.t_end = t +
          _elm_config->page_scroll_friction;

        sd->g_layer_zoom.bounce.animator =
          ecore_animator_add(_bounce_eval, obj);
     }
   else
     {
        eo_do(obj, elm_scrollable_interface_freeze_set(EINA_FALSE));
        sd->zoom_g_layer = EINA_FALSE;
     }

   return EVAS_EVENT_FLAG_NONE;
}

static void
_elm_photocam_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Photocam_Smart_Data *priv = _pd;
   Eina_Bool bounce = _elm_config->thumbscroll_bounce_enable;
   Elm_Photocam_Pan_Smart_Data *pan_data;
   Evas_Object *edje;
   Evas_Coord minw, minh;

   elm_widget_sub_object_parent_add(obj);

   edje = edje_object_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, edje);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());

   elm_widget_theme_object_set
      (obj, edje, "photocam", "base", elm_widget_style_get(obj));

   priv->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(priv->hit_rect, obj);
   elm_widget_sub_object_add(obj, priv->hit_rect);

   /* common scroller hit rectangle setup */
   evas_object_color_set(priv->hit_rect, 0, 0, 0, 0);
   evas_object_show(priv->hit_rect);
   evas_object_repeat_events_set(priv->hit_rect, EINA_TRUE);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   eo_do(obj, elm_scrollable_interface_objects_set(edje, priv->hit_rect));

   eo_do(obj,
         elm_scrollable_interface_animate_start_cb_set(_scroll_animate_start_cb),
         elm_scrollable_interface_animate_stop_cb_set(_scroll_animate_stop_cb),
         elm_scrollable_interface_drag_start_cb_set(_scroll_drag_start_cb),
         elm_scrollable_interface_drag_stop_cb_set(_scroll_drag_stop_cb),
         elm_scrollable_interface_scroll_cb_set(_scroll_cb));

   eo_do(obj, elm_scrollable_interface_bounce_allow_set(bounce, bounce));

   priv->pan_obj = eo_add(MY_PAN_CLASS, evas_object_evas_get(obj));
   pan_data = eo_data_scope_get(priv->pan_obj, MY_PAN_CLASS);
   eo_data_ref(obj, NULL);
   pan_data->wobj = obj;
   pan_data->wsd = priv;

   eo_do(obj, elm_scrollable_interface_extern_pan_set(priv->pan_obj));

   priv->g_layer_start = 1.0;
   priv->zoom = 1;
   priv->mode = ELM_PHOTOCAM_ZOOM_MODE_MANUAL;
   priv->tsize = 512;

   priv->img = evas_object_image_add(evas_object_evas_get(obj));
   evas_object_image_load_orientation_set(priv->img, EINA_TRUE);
   evas_object_image_scale_hint_set(priv->img, EVAS_IMAGE_SCALE_HINT_DYNAMIC);
   evas_object_event_callback_add
     (priv->img, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, obj);
   evas_object_event_callback_add
     (priv->img, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, obj);
   evas_object_image_scale_hint_set(priv->img, EVAS_IMAGE_SCALE_HINT_STATIC);

   /* XXX: mmm... */
   evas_object_smart_member_add(priv->img, priv->pan_obj);

   elm_widget_sub_object_add(obj, priv->img);
   evas_object_image_filled_set(priv->img, EINA_TRUE);
   evas_object_event_callback_add
     (priv->img, EVAS_CALLBACK_IMAGE_PRELOADED, _main_img_preloaded_cb, obj);

   edje_object_size_min_calc(edje, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   _sizing_eval(obj);

}

static void
_elm_photocam_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Phocam_Grid *g;

   Elm_Photocam_Smart_Data *sd = _pd;

   EINA_LIST_FREE(sd->grids, g)
     {
        if (g->grid) free(g->grid);
        free(g);
     }
   eo_unref(sd->pan_obj);
   evas_object_del(sd->pan_obj);
   sd->pan_obj = NULL;

   if (sd->f) eina_file_close(sd->f);
   free(sd->remote_data);
   if (sd->remote) elm_url_cancel(sd->remote);
   if (sd->file) eina_stringshare_del(sd->file);
   if (sd->calc_job) ecore_job_del(sd->calc_job);
   if (sd->scr_timer) ecore_timer_del(sd->scr_timer);
   if (sd->long_timer) ecore_timer_del(sd->long_timer);
   if (sd->zoom_animator) ecore_animator_del(sd->zoom_animator);
   if (sd->g_layer_zoom.bounce.animator)
     ecore_animator_del(sd->g_layer_zoom.bounce.animator);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

static void
_elm_photocam_smart_move(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   Elm_Photocam_Smart_Data *sd = _pd;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_move(x, y));

   evas_object_move(sd->hit_rect, x, y);
}

static void
_elm_photocam_smart_resize(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);
   Elm_Photocam_Smart_Data *sd = _pd;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_resize(w, h));

   evas_object_resize(sd->hit_rect, w, h);
}

static void
_elm_photocam_smart_member_add(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *member = va_arg(*list, Evas_Object *);
   Elm_Photocam_Smart_Data *sd = _pd;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_member_add(member));

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

EAPI Evas_Object *
elm_photocam_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EAPI Evas_Load_Error
elm_photocam_file_set(Evas_Object *obj,
                      const char *file)
{
   ELM_PHOTOCAM_CHECK(obj) EVAS_LOAD_ERROR_NONE;
   Evas_Load_Error ret = EVAS_LOAD_ERROR_NONE;
   eo_do(obj, elm_obj_photocam_file_set(file, &ret));
   return ret;
}

static void
_internal_file_set(Eo *obj, Elm_Photocam_Smart_Data *sd, const char *file, Eina_File *f, Evas_Load_Error *ret)
{
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);
   Evas_Load_Error err;
   int w, h;
   double tz;

   if (!eina_stringshare_replace(&sd->file, file)) return;
   sd->f = eina_file_dup(f);

   evas_object_image_smooth_scale_set(sd->img, (sd->no_smooth == 0));
   evas_object_image_file_set(sd->img, NULL, NULL);
   evas_object_image_load_scale_down_set(sd->img, 0);
   _photocam_image_file_set(sd->img, sd);
   err = evas_object_image_load_error_get(sd->img);
   if (err != EVAS_LOAD_ERROR_NONE)
     {
        ERR("Things are going bad for '%s' (%p) : %i", file, sd->img, err);
        if (ret) *ret = err;
        return;
     }
   evas_object_image_size_get(sd->img, &w, &h);

   sd->do_region = evas_object_image_region_support_get(sd->img);
   sd->size.imw = w;
   sd->size.imh = h;
   sd->size.w = sd->size.imw / sd->zoom;
   sd->size.h = sd->size.imh / sd->zoom;
   evas_object_image_file_set(sd->img, NULL, NULL);
   evas_object_image_load_scale_down_set(sd->img, 8);
   _photocam_image_file_set(sd->img, sd);
   err = evas_object_image_load_error_get(sd->img);
   if (err != EVAS_LOAD_ERROR_NONE)
     {
        ERR("Things are going bad for '%s' (%p)", file, sd->img);
        if (ret) *ret = err;
        return;
     }

   evas_object_image_preload(sd->img, 0);
   sd->main_load_pending = EINA_TRUE;

   sd->calc_job = ecore_job_add(_calc_job_cb, obj);
   evas_object_smart_callback_call(obj, SIG_LOAD, NULL);
   sd->preload_num++;
   if (sd->preload_num == 1)
     {
        edje_object_signal_emit
          (wd->resize_obj, "elm,state,busy,start", "elm");
        evas_object_smart_callback_call(obj, SIG_LOAD_DETAIL, NULL);
     }

   tz = sd->zoom;
   sd->zoom = 0.0;
   elm_photocam_zoom_set(obj, tz);

   if (ret) *ret = evas_object_image_load_error_get(sd->img);
}

static void
_elm_photocam_download_done(void *data, Elm_Url *url EINA_UNUSED, Eina_Binbuf *download)
{
   Eo *obj = data;
   Elm_Photocam_Smart_Data *sd = eo_data_scope_get(obj, MY_CLASS);
   Eina_File *f;
   size_t length;
   Evas_Load_Error ret = EVAS_LOAD_ERROR_NONE;

   if (sd->remote_data) free(sd->remote_data);
   length = eina_binbuf_length_get(download);
   sd->remote_data = eina_binbuf_string_steal(download);
   f = eina_file_virtualize(elm_url_get(url),
                            sd->remote_data, length,
                            EINA_FALSE);
   _internal_file_set(obj, sd, elm_url_get(url), f, &ret);
   eina_file_close(f);

   if (ret != EVAS_LOAD_ERROR_NONE)
     {
        Elm_Photocam_Error err = { 0, EINA_TRUE };

        free(sd->remote_data);
        sd->remote_data = NULL;
        evas_object_smart_callback_call(obj, SIG_DOWNLOAD_ERROR, &err);
     }
   else
     {
        evas_object_smart_callback_call(obj, SIG_DOWNLOAD_DONE, NULL);
     }

   sd->remote = NULL;
}

static void
_elm_photocam_download_cancel(void *data, Elm_Url *url EINA_UNUSED, int error)
{
   Eo *obj = data;
   Elm_Photocam_Smart_Data *sd = eo_data_scope_get(obj, MY_CLASS);
   Elm_Photocam_Error err = { error, EINA_FALSE };

   evas_object_smart_callback_call(obj, SIG_DOWNLOAD_ERROR, &err);

   sd->remote = NULL;
}


static void
_elm_photocam_download_progress(void *data, Elm_Url *url EINA_UNUSED, double now, double total)
{
   Eo *obj = data;
   Elm_Photocam_Progress progress;

   progress.now = now;
   progress.total = total;
   evas_object_smart_callback_call(obj, SIG_DOWNLOAD_PROGRESS, &progress);
}


static const char *remote_uri[] = {
  "http://", "https://", "ftp://"
};

static void
_file_set(Eo *obj, void *_pd, va_list *list)
{
   const char *file = va_arg(*list, const char *);
   Evas_Load_Error *ret = va_arg(*list, Evas_Load_Error *);
   if (ret) *ret = EVAS_LOAD_ERROR_NONE;

   Elm_Photocam_Smart_Data *sd = _pd;
   unsigned int i;

   _grid_clear_all(obj);
   ELM_SAFE_FREE(sd->g_layer_zoom.bounce.animator, ecore_animator_del);
   if (sd->zoom_animator)
     {
        sd->no_smooth--;
        if (sd->no_smooth == 0) _smooth_update(obj);
        ecore_animator_del(sd->zoom_animator);
        sd->zoom_animator = NULL;
     }
   if (sd->calc_job) ecore_job_del(sd->calc_job);
   evas_object_hide(sd->img);
   if (sd->f) eina_file_close(sd->f);
   sd->f = NULL;

   free(sd->remote_data);
   if (sd->remote) elm_url_cancel(sd->remote);
   sd->remote = NULL;

   for (i = 0; i < sizeof (remote_uri) / sizeof (remote_uri[0]); ++i)
     if (strncmp(remote_uri[i], file, strlen(remote_uri[i])) == 0)
       {
          // Found a remote target !
          sd->remote = elm_url_download(file,
                                        _elm_photocam_download_done,
                                        _elm_photocam_download_cancel,
                                        _elm_photocam_download_progress,
                                        obj);
          if (sd->remote)
            {
               evas_object_smart_callback_call(obj, SIG_DOWNLOAD_START, NULL);
               return ;
            }
          break;
       }


   _internal_file_set(obj, sd, file, NULL, ret);
}

EAPI const char *
elm_photocam_file_get(const Evas_Object *obj)
{
   ELM_PHOTOCAM_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_photocam_file_get(&ret));
   return ret;
}

static void
_file_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   Elm_Photocam_Smart_Data *sd = _pd;

   *ret = sd->file;
}

EAPI void
elm_photocam_zoom_set(Evas_Object *obj,
                      double zoom)
{
   ELM_PHOTOCAM_CHECK(obj);
   eo_do(obj, elm_obj_photocam_zoom_set(zoom));
}

static void
_zoom_set(Eo *obj, void *_pd, va_list *list)
{
   double z;
   Eina_List *l;
   Ecore_Animator *an;
   Elm_Phocam_Grid *g, *g_zoom = NULL;
   Evas_Coord pw, ph, rx, ry, rw, rh;
   int zoom_changed = 0, started = 0;

   double zoom = va_arg(*list, double);
   Elm_Photocam_Smart_Data *sd = _pd;

   if (zoom <= (1.0 / 256.0)) zoom = (1.0 / 256.0);
   if (zoom == sd->zoom) return;

   sd->zoom = zoom;
   sd->size.ow = sd->size.w;
   sd->size.oh = sd->size.h;
   eo_do(obj, elm_scrollable_interface_content_pos_get(&rx, &ry));
   eo_do(obj, elm_scrollable_interface_content_viewport_size_get(&rw, &rh));
   if ((rw <= 0) || (rh <= 0)) return;

   if (sd->mode == ELM_PHOTOCAM_ZOOM_MODE_MANUAL)
     {
        sd->size.nw = (double)sd->size.imw / sd->zoom;
        sd->size.nh = (double)sd->size.imh / sd->zoom;
     }
   else if (sd->mode == ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT)
     {
        if ((sd->size.imw < 1) || (sd->size.imh < 1))
          {
             sd->size.nw = 0;
             sd->size.nh = 0;
          }
        else
          {
             ph = (sd->size.imh * rw) / sd->size.imw;
             if (ph > rh)
               {
                  pw = (sd->size.imw * rh) / sd->size.imh;
                  ph = rh;
               }
             else
               {
                  pw = rw;
               }
             if (sd->size.imw > sd->size.imh)
               z = (double)sd->size.imw / pw;
             else
               z = (double)sd->size.imh / ph;
             if (z != sd->zoom)
               zoom_changed = 1;
             sd->zoom = z;
             sd->size.nw = pw;
             sd->size.nh = ph;
          }
     }
   else if (sd->mode == ELM_PHOTOCAM_ZOOM_MODE_AUTO_FILL)
     {
        if ((sd->size.imw < 1) || (sd->size.imh < 1))
          {
             sd->size.nw = 0;
             sd->size.nw = 0;
          }
        else
          {
             ph = (sd->size.imh * rw) / sd->size.imw;
             if (ph < rh)
               {
                  pw = (sd->size.imw * rh) / sd->size.imh;
                  ph = rh;
               }
             else
               {
                  pw = rw;
               }
             if (sd->size.imw > sd->size.imh)
               z = (double)sd->size.imw / pw;
             else
               z = (double)sd->size.imh / ph;
             if (z != sd->zoom)
               zoom_changed = 1;
             sd->zoom = z;
             sd->size.nw = pw;
             sd->size.nh = ph;
          }
     }
   else if (sd->mode == ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT_IN)
     {
        if ((sd->size.imw < 1) || (sd->size.imh < 1))
          {
             sd->size.nw = 0;
             sd->size.nh = 0;
          }
        else if ((sd->size.imw < rw) && (sd->size.imh < rh))
          {
             if (1 != sd->zoom) zoom_changed = 1;
             sd->zoom = 1;
             sd->size.nw = sd->size.imw;
             sd->size.nh = sd->size.imh;
          }
        else
          {
             ph = (sd->size.imh * rw) / sd->size.imw;
             if (ph > rh)
               {
                  pw = (sd->size.imw * rh) / sd->size.imh;
                  ph = rh;
               }
             else
               pw = rw;
             if (sd->size.imw > sd->size.imh)
               z = (double)sd->size.imw / pw;
             else
               z = (double)sd->size.imh / ph;
             if (z != sd->zoom)
               zoom_changed = 1;
             sd->zoom = z;
             sd->size.nw = pw;
             sd->size.nh = ph;
          }
     }

   if (sd->main_load_pending)
     {
        sd->size.w = sd->size.nw;
        sd->size.h = sd->size.nh;

        goto done;
     }

   EINA_LIST_FOREACH(sd->grids, l, g)
     {
        if (g->zoom == _grid_zoom_calc(sd->zoom))
          {
             sd->grids = eina_list_remove(sd->grids, g);
             sd->grids = eina_list_prepend(sd->grids, g);
             _grid_raise(g);
             goto done;
          }
     }

   g = _grid_create(obj);
   if (g)
     {
        if (eina_list_count(sd->grids) > 1)
          {
             g_zoom = eina_list_last(sd->grids)->data;
             sd->grids = eina_list_remove(sd->grids, g_zoom);
             _grid_clear(obj, g_zoom);
             free(g_zoom);
             EINA_LIST_FOREACH(sd->grids, l, g_zoom)
               {
                  g_zoom->dead = 1;
               }
          }
        sd->grids = eina_list_prepend(sd->grids, g);
     }
   else
     {
        EINA_LIST_FREE(sd->grids, g)
          {
             _grid_clear(obj, g);
             free(g);
          }
     }

done:
   sd->t_start = ecore_loop_time_get();
   sd->t_end = sd->t_start + _elm_config->zoom_friction;
   if ((sd->size.w > 0) && (sd->size.h > 0))
     {
        sd->size.spos.x = (double)(rx + (rw / 2)) / (double)sd->size.w;
        sd->size.spos.y = (double)(ry + (rh / 2)) / (double)sd->size.h;
     }
   else
     {
        sd->size.spos.x = 0.5;
        sd->size.spos.y = 0.5;
     }
   if (rw > sd->size.w) sd->size.spos.x = 0.5;
   if (rh > sd->size.h) sd->size.spos.y = 0.5;
   if (sd->size.spos.x > 1.0) sd->size.spos.x = 1.0;
   if (sd->size.spos.y > 1.0) sd->size.spos.y = 1.0;

   if (sd->paused)
     {
        _zoom_do(obj, 1.0);
     }
   else
     {
        if (!sd->zoom_animator)
          {
             sd->zoom_animator = ecore_animator_add(_zoom_anim_cb, obj);
             sd->no_smooth++;
             if (sd->no_smooth == 1) _smooth_update(obj);
             started = 1;
          }
     }

   an = sd->zoom_animator;
   if (an)
     {
        if (!_zoom_anim_cb(obj))
          {
             ecore_animator_del(an);
             an = NULL;
          }
     }

   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job_cb, obj);
   if (!sd->paused)
     {
        if (started)
          evas_object_smart_callback_call(obj, SIG_ZOOM_START, NULL);
        if (!an)
          evas_object_smart_callback_call(obj, SIG_ZOOM_STOP, NULL);
     }
   if (zoom_changed)
     evas_object_smart_callback_call(obj, SIG_ZOOM_CHANGE, NULL);
}

EAPI double
elm_photocam_zoom_get(const Evas_Object *obj)
{
   ELM_PHOTOCAM_CHECK(obj) 1.0;
   double ret = 1.0;
   eo_do((Eo *) obj, elm_obj_photocam_zoom_get(&ret));
   return ret;
}

static void
_zoom_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   Elm_Photocam_Smart_Data *sd = _pd;

   *ret = sd->zoom;
}

EAPI void
elm_photocam_zoom_mode_set(Evas_Object *obj,
                           Elm_Photocam_Zoom_Mode mode)
{
   ELM_PHOTOCAM_CHECK(obj);
   eo_do(obj, elm_obj_photocam_zoom_mode_set(mode));
}

static void
_zoom_mode_set(Eo *obj, void *_pd, va_list *list)
{
   double tz;

   Elm_Photocam_Zoom_Mode mode = va_arg(*list, Elm_Photocam_Zoom_Mode);
   Elm_Photocam_Smart_Data *sd = _pd;

   if (sd->mode == mode) return;
   sd->mode = mode;

   tz = sd->zoom;
   sd->zoom = 0.0;
   elm_photocam_zoom_set(obj, tz);
}

EAPI Elm_Photocam_Zoom_Mode
elm_photocam_zoom_mode_get(const Evas_Object *obj)
{
   ELM_PHOTOCAM_CHECK(obj) ELM_PHOTOCAM_ZOOM_MODE_LAST;
   Elm_Photocam_Zoom_Mode ret = ELM_PHOTOCAM_ZOOM_MODE_LAST;
   eo_do((Eo *) obj, elm_obj_photocam_zoom_mode_get(&ret));
   return ret;
}

static void
_zoom_mode_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Photocam_Zoom_Mode *ret = va_arg(*list, Elm_Photocam_Zoom_Mode *);
   Elm_Photocam_Smart_Data *sd = _pd;

   *ret = sd->mode;
}

EAPI void
elm_photocam_image_size_get(const Evas_Object *obj,
                            int *w,
                            int *h)
{
   ELM_PHOTOCAM_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_photocam_image_size_get(w, h));
}

static void
_image_size_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *w = va_arg(*list, int *);
   int *h = va_arg(*list, int *);
   Elm_Photocam_Smart_Data *sd = _pd;

   if (w) *w = sd->size.imw;
   if (h) *h = sd->size.imh;
}

EAPI void
elm_photocam_image_region_get(const Evas_Object *obj,
                              int *x,
                              int *y,
                              int *w,
                              int *h)
{
   ELM_PHOTOCAM_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_photocam_image_region_get(x, y, w, h));
}

static void
_image_region_get(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord sx, sy, sw, sh;

   int *x = va_arg(*list, int *);
   int *y = va_arg(*list, int *);
   int *w = va_arg(*list, int *);
   int *h = va_arg(*list, int *);
   Elm_Photocam_Smart_Data *sd = _pd;

   eo_do((Eo *)obj, elm_scrollable_interface_content_pos_get(&sx, &sy));
   eo_do((Eo *)obj, elm_scrollable_interface_content_viewport_size_get(&sw, &sh));
   if (sd->size.w > 0)
     {
        if (x)
          {
             *x = (sd->size.imw * sx) / sd->size.w;
             if (*x > sd->size.imw) *x = sd->size.imw;
          }
        if (w)
          {
             *w = (sd->size.imw * sw) / sd->size.w;
             if (*w > sd->size.imw) *w = sd->size.imw;
             else if (*w < 0)
               *w = 0;
          }
     }
   else
     {
        if (x) *x = 0;
        if (w) *w = 0;
     }

   if (sd->size.h > 0)
     {
        if (y)
          {
             *y = (sd->size.imh * sy) / sd->size.h;
             if (*y > sd->size.imh) *y = sd->size.imh;
          }
        if (h)
          {
             *h = (sd->size.imh * sh) / sd->size.h;
             if (*h > sd->size.imh) *h = sd->size.imh;
             else if (*h < 0)
               *h = 0;
          }
     }
   else
     {
        if (y) *y = 0;
        if (h) *h = 0;
     }
}

EAPI void
elm_photocam_image_region_show(Evas_Object *obj,
                               int x,
                               int y,
                               int w,
                               int h)
{
   ELM_PHOTOCAM_CHECK(obj);
   eo_do(obj, elm_obj_photocam_image_region_show(x, y, w, h));
}

static void
_image_region_show(Eo *obj, void *_pd, va_list *list)
{
   int rx, ry, rw, rh;

   int x = va_arg(*list, int);
   int y = va_arg(*list, int);
   int w = va_arg(*list, int);
   int h = va_arg(*list, int);
   Elm_Photocam_Smart_Data *sd = _pd;

   if ((sd->size.imw < 1) || (sd->size.imh < 1)) return;
   rx = (x * sd->size.w) / sd->size.imw;
   ry = (y * sd->size.h) / sd->size.imh;
   rw = (w * sd->size.w) / sd->size.imw;
   rh = (h * sd->size.h) / sd->size.imh;
   if (rw < 1) rw = 1;
   if (rh < 1) rh = 1;
   if ((rx + rw) > sd->size.w) rx = sd->size.w - rw;
   if ((ry + rh) > sd->size.h) ry = sd->size.h - rh;
   if (sd->g_layer_zoom.bounce.animator)
     {
        ecore_animator_del(sd->g_layer_zoom.bounce.animator);
        sd->g_layer_zoom.bounce.animator = NULL;
        _zoom_do(obj, 1.0);
     }
   if (sd->zoom_animator)
     {
        sd->no_smooth--;
        ecore_animator_del(sd->zoom_animator);
        sd->zoom_animator = NULL;
        _zoom_do(obj, 1.0);
        evas_object_smart_callback_call(obj, SIG_ZOOM_STOP, NULL);
     }
   eo_do(obj, elm_scrollable_interface_content_region_show(rx, ry, rw, rh));
}

EAPI void
elm_photocam_image_region_bring_in(Evas_Object *obj,
                                   int x,
                                   int y,
                                   int w,
                                   int h __UNUSED__)
{
   ELM_PHOTOCAM_CHECK(obj);
   eo_do(obj, elm_scrollable_interface_region_bring_in(x, y, w, h));
}

static void
_image_region_bring_in(Eo *obj, void *_pd, va_list *list)
{
   int x = va_arg(*list, int);
   int y = va_arg(*list, int);
   int w = va_arg(*list, int);
   int h = va_arg(*list, int);
   int rx, ry, rw, rh;

   Elm_Photocam_Smart_Data *sd = _pd;

   if ((sd->size.imw < 1) || (sd->size.imh < 1)) return;
   rx = (x * sd->size.w) / sd->size.imw;
   ry = (y * sd->size.h) / sd->size.imh;
   rw = (w * sd->size.w) / sd->size.imw;
   rh = (h * sd->size.h) / sd->size.imh;
   if (rw < 1) rw = 1;
   if (rh < 1) rh = 1;
   if ((rx + rw) > sd->size.w) rx = sd->size.w - rw;
   if ((ry + rh) > sd->size.h) ry = sd->size.h - rh;
   if (sd->g_layer_zoom.bounce.animator)
     {
        ecore_animator_del(sd->g_layer_zoom.bounce.animator);
        sd->g_layer_zoom.bounce.animator = NULL;
        _zoom_do(obj, 1.0);
     }
   if (sd->zoom_animator)
     {
        sd->no_smooth--;
        if (!sd->no_smooth) _smooth_update(obj);
        ecore_animator_del(sd->zoom_animator);
        sd->zoom_animator = NULL;
        _zoom_do(obj, 1.0);
        evas_object_smart_callback_call(obj, SIG_ZOOM_STOP, NULL);
     }
   eo_do_super(obj, MY_CLASS, elm_scrollable_interface_region_bring_in(rx, ry, rw, rh));
}

EAPI void
elm_photocam_paused_set(Evas_Object *obj,
                        Eina_Bool paused)
{
   ELM_PHOTOCAM_CHECK(obj);
   eo_do(obj, elm_obj_photocam_paused_set(paused));
}

static void
_paused_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool paused = va_arg(*list, int);
   Elm_Photocam_Smart_Data *sd = _pd;

   paused = !!paused;

   if (sd->paused == paused) return;
   sd->paused = paused;
   if (!sd->paused) return;

   if (sd->g_layer_zoom.bounce.animator)
     {
        ecore_animator_del(sd->g_layer_zoom.bounce.animator);
        sd->g_layer_zoom.bounce.animator = NULL;
        _zoom_do(obj, 1.0);
     }
   if (sd->zoom_animator)
     {
        ecore_animator_del(sd->zoom_animator);
        sd->zoom_animator = NULL;
        _zoom_do(obj, 1.0);
        evas_object_smart_callback_call(obj, SIG_ZOOM_STOP, NULL);
     }
}

EAPI Eina_Bool
elm_photocam_paused_get(const Evas_Object *obj)
{
   ELM_PHOTOCAM_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_photocam_paused_get(&ret));
   return ret;
}

static void
_paused_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Photocam_Smart_Data *sd = _pd;

   *ret = sd->paused;
}

EAPI Evas_Object *
elm_photocam_internal_image_get(const Evas_Object *obj)
{
   ELM_PHOTOCAM_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_obj_photocam_internal_image_get(&ret));
   return ret;
}

static void
_internal_image_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Elm_Photocam_Smart_Data *sd = _pd;

   *ret = sd->img;
}

EAPI void
elm_photocam_bounce_set(Evas_Object *obj,
                        Eina_Bool h_bounce,
                        Eina_Bool v_bounce)
{
   ELM_PHOTOCAM_CHECK(obj);

   eo_do(obj, elm_scrollable_interface_bounce_allow_set(h_bounce, v_bounce));
}

EAPI void
elm_photocam_bounce_get(const Evas_Object *obj,
                        Eina_Bool *h_bounce,
                        Eina_Bool *v_bounce)
{
   ELM_PHOTOCAM_CHECK(obj);

   eo_do((Eo *)obj, elm_scrollable_interface_bounce_allow_get(h_bounce, v_bounce));
}

EAPI void
elm_photocam_gesture_enabled_set(Evas_Object *obj,
                                 Eina_Bool gesture)
{
   ELM_PHOTOCAM_CHECK(obj);
   eo_do(obj, elm_obj_photocam_gesture_enabled_set(gesture));
}

static void
_gesture_enabled_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool gesture = va_arg(*list, int);
   Elm_Photocam_Smart_Data *sd = _pd;

   gesture = !!gesture;

   if (sd->do_gesture == gesture) return;

   sd->do_gesture = gesture;

   if (sd->g_layer)
     {
        evas_object_del(sd->g_layer);
        sd->g_layer = NULL;
     }

   if (!gesture) return;

   sd->g_layer = elm_gesture_layer_add(obj);
   if (!sd->g_layer) return;

   elm_gesture_layer_attach(sd->g_layer, obj);
   elm_gesture_layer_cb_set
     (sd->g_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_START,
     _g_layer_zoom_start_cb, obj);
   elm_gesture_layer_cb_set
     (sd->g_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_MOVE,
     _g_layer_zoom_move_cb, obj);
   elm_gesture_layer_cb_set
     (sd->g_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_END,
     _g_layer_zoom_end_cb, obj);
   elm_gesture_layer_cb_set
     (sd->g_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_ABORT,
     _g_layer_zoom_end_cb, obj);
}

EAPI Eina_Bool
elm_photocam_gesture_enabled_get(const Evas_Object *obj)
{
   ELM_PHOTOCAM_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_photocam_gesture_enabled_get(&ret));
   return ret;
}

static void
_gesture_enabled_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Photocam_Smart_Data *sd = _pd;

   *ret = sd->do_gesture;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MEMBER_ADD), _elm_photocam_smart_member_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_photocam_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_photocam_smart_del),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_RESIZE), _elm_photocam_smart_resize),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MOVE), _elm_photocam_smart_move),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_THEME), _elm_photocam_smart_theme),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_EVENT), _elm_photocam_smart_event),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ON_FOCUS), _elm_photocam_smart_on_focus),

        EO_OP_FUNC(ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_REGION_BRING_IN), _image_region_bring_in),

        EO_OP_FUNC(ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_FILE_SET), _file_set),
        EO_OP_FUNC(ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_FILE_GET), _file_get),
        EO_OP_FUNC(ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_ZOOM_SET), _zoom_set),
        EO_OP_FUNC(ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_ZOOM_GET), _zoom_get),
        EO_OP_FUNC(ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_ZOOM_MODE_SET), _zoom_mode_set),
        EO_OP_FUNC(ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_ZOOM_MODE_GET), _zoom_mode_get),
        EO_OP_FUNC(ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_IMAGE_SIZE_GET), _image_size_get),
        EO_OP_FUNC(ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_IMAGE_REGION_GET), _image_region_get),
        EO_OP_FUNC(ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_IMAGE_REGION_SHOW), _image_region_show),
        EO_OP_FUNC(ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_PAUSED_SET), _paused_set),
        EO_OP_FUNC(ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_PAUSED_GET), _paused_get),
        EO_OP_FUNC(ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_INTERNAL_IMAGE_GET), _internal_image_get),
        EO_OP_FUNC(ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_GESTURE_ENABLED_SET), _gesture_enabled_set),
        EO_OP_FUNC(ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_GESTURE_ENABLED_GET), _gesture_enabled_get),

        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_PHOTOCAM_SUB_ID_FILE_SET, "Set the photo file to be shown."),
     EO_OP_DESCRIPTION(ELM_OBJ_PHOTOCAM_SUB_ID_FILE_GET, "Returns the path of the current image file."),
     EO_OP_DESCRIPTION(ELM_OBJ_PHOTOCAM_SUB_ID_ZOOM_SET, "Set the zoom level of the photo."),
     EO_OP_DESCRIPTION(ELM_OBJ_PHOTOCAM_SUB_ID_ZOOM_GET, "Get the zoom level of the photo."),
     EO_OP_DESCRIPTION(ELM_OBJ_PHOTOCAM_SUB_ID_ZOOM_MODE_SET, "Set the zoom mode."),
     EO_OP_DESCRIPTION(ELM_OBJ_PHOTOCAM_SUB_ID_ZOOM_MODE_GET, "Get the zoom mode."),
     EO_OP_DESCRIPTION(ELM_OBJ_PHOTOCAM_SUB_ID_IMAGE_SIZE_GET, "Get the current image pixel width and height."),
     EO_OP_DESCRIPTION(ELM_OBJ_PHOTOCAM_SUB_ID_IMAGE_REGION_GET, "Get the region of the image that is currently shown."),
     EO_OP_DESCRIPTION(ELM_OBJ_PHOTOCAM_SUB_ID_IMAGE_REGION_SHOW, "Set the viewed region of the image."),
     EO_OP_DESCRIPTION(ELM_OBJ_PHOTOCAM_SUB_ID_PAUSED_SET, "Set the paused state for photocam."),
     EO_OP_DESCRIPTION(ELM_OBJ_PHOTOCAM_SUB_ID_PAUSED_GET, "Get the paused state for photocam."),
     EO_OP_DESCRIPTION(ELM_OBJ_PHOTOCAM_SUB_ID_INTERNAL_IMAGE_GET, "Get the internal low-res image used for photocam."),
     EO_OP_DESCRIPTION(ELM_OBJ_PHOTOCAM_SUB_ID_GESTURE_ENABLED_SET, "Set the gesture state for photocam."),
     EO_OP_DESCRIPTION(ELM_OBJ_PHOTOCAM_SUB_ID_GESTURE_ENABLED_GET, "Get the gesture state for photocam."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_PHOTOCAM_BASE_ID, op_desc, ELM_OBJ_PHOTOCAM_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Photocam_Smart_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_photocam_class_get, &class_desc, ELM_OBJ_WIDGET_CLASS, ELM_SCROLLABLE_INTERFACE, NULL);

