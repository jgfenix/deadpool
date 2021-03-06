#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#include "elm_priv.h"
#include "elm_model_tree.h"

#include "elm_model_tree_private.h"

#include <assert.h>

#define MY_CONST_CLASS ELM_OBJ_MODEL_TREE_CONST_CLASS
EAPI Eo_Op ELM_OBJ_MODEL_TREE_CONST_BASE_ID = 0;

EAPI const Eo_Event_Description _ELM_MODEL_TREE_CONST_SELECT_EVT =
   EO_EVENT_DESCRIPTION("tree const, select","Select the node pointed by path.");

struct _Elm_Model_Tree
{
   Elm_Model_Tree_Node *root;
   Elm_Model_Tree_Path* selected;
   Eina_Lock lock;
};

typedef struct _Elm_Model_Tree Elm_Model_Tree;


/*
 * Const Class definition
 */
static void
_model_tree_constructor(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   Elm_Model_Tree_Node *root;
   Elm_Model_Tree *model = class_data;
   Eina_Value *value = va_arg(*list, Eina_Value *);

   eo_do_super(obj, MY_CONST_CLASS, eo_constructor());

   EINA_SAFETY_ON_NULL_RETURN(model);
   root = _tree_node_append(value, NULL);
   EINA_SAFETY_ON_NULL_RETURN(root);
   model->root = root;
   model->selected = NULL;
   eina_lock_new(&model->lock);
}

static void
_model_tree_destructor(Eo *obj EINA_UNUSED, void *class_data, va_list *list EINA_UNUSED)
{
   Elm_Model_Tree *model = class_data;

   EINA_SAFETY_ON_NULL_RETURN(model);
   eina_lock_take(&model->lock);
   _tree_node_del(model->root);
   elm_model_tree_path_free(model->selected);
   eina_lock_release(&model->lock);
   eina_lock_free(&model->lock);
   eo_do_super(obj, MY_CONST_CLASS, eo_destructor());
}

static void
_model_tree_select(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   Elm_Model_Tree_Node *node;
   Elm_Model_Tree *model = class_data;
   Elm_Model_Tree_Path *path = va_arg(*list, Elm_Model_Tree_Path *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);

   EINA_SAFETY_ON_NULL_RETURN(model);
   eina_lock_take(&model->lock);
   node = _tree_node_find(model->root, path);
   model->selected = node ? path : NULL;
   eina_lock_release(&model->lock);
   eo_do(obj, eo_event_callback_call(ELM_MODEL_TREE_CONST_SELECT_EVT, ret, NULL));

   if (node) *ret = EINA_TRUE;
   else *ret = EINA_FALSE;
}

static void
_model_tree_value_get(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   Elm_Model_Tree_Node *node;

   Elm_Model_Tree *model = class_data;
   Elm_Model_Tree_Path *path = va_arg(*list, Elm_Model_Tree_Path *);
   Eina_Value **value = va_arg(*list, Eina_Value **);

   EINA_SAFETY_ON_NULL_GOTO(path, exit_err);
   EINA_SAFETY_ON_NULL_GOTO(model, exit_err);

   eina_lock_take(&model->lock);

   node = _tree_node_find(model->root, path);
   if(!node)
     {
        eina_lock_release(&model->lock);
        EINA_SAFETY_ON_NULL_GOTO(NULL, exit_err);
     }

   *value = _tree_node_value_get(node);

   eina_lock_release(&model->lock);
   return;

exit_err:
   *value = NULL;
}

static void
_model_tree_children_get(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED, va_list *list)
{
   (void)va_arg(*list, Elm_Model_Tree_Path *);
   Eina_List **ret = va_arg(*list, Eina_List **);
   *ret = NULL; //TODO: Implement this
}

static void
_model_tree_selected_get(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   Elm_Model_Tree *model = class_data;
   EINA_SAFETY_ON_NULL_RETURN(model);
   Elm_Model_Tree_Path **ret = va_arg(*list, Elm_Model_Tree_Path **);
   *ret = model->selected;
}

/* Eina_Bool */
/* _model_tree_release_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED) */
/* { */
/*    //TODO: Implement this */
/*    return EINA_FALSE; */
/* } */


//static void
//_model_tree_release(Eo *object, void *_unused EINA_UNUSED, Elm_Model_Tree_Path *path)
static void
_model_tree_release(Eo *obj, void *class_data EINA_UNUSED, va_list *list)
{
   Elm_Model_Tree_Node *node;
   Elm_Model_Tree_Path *path = va_arg(*list, Elm_Model_Tree_Path *);

   Elm_Model_Tree *model = eo_data_scope_get(obj, MY_CONST_CLASS);
   if(path == NULL) return;
   EINA_SAFETY_ON_NULL_RETURN(model);
   eina_lock_take(&model->lock);
   node = _tree_node_find(model->root, path);
   EINA_SAFETY_ON_NULL_RETURN(node);
   _tree_node_del(node);
   eina_lock_release(&model->lock);
   //eo2_do(object, elm_model_tree_delete_callback_call(path));
}

static void
_model_tree_const_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_descs[] = {
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _model_tree_constructor),
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _model_tree_destructor),
      EO_OP_FUNC(ELM_OBJ_MODEL_TREE_CONST_ID(ELM_OBJ_MODEL_TREE_CONST_SUB_ID_SELECT), _model_tree_select),
      EO_OP_FUNC(ELM_OBJ_MODEL_TREE_CONST_ID(ELM_OBJ_MODEL_TREE_CONST_SUB_ID_VALUE_GET), _model_tree_value_get),
      EO_OP_FUNC(ELM_OBJ_MODEL_TREE_CONST_ID(ELM_OBJ_MODEL_TREE_CONST_SUB_ID_CHILDREN_GET), _model_tree_children_get),
      EO_OP_FUNC(ELM_OBJ_MODEL_TREE_CONST_ID(ELM_OBJ_MODEL_TREE_CONST_SUB_ID_SELECTED_GET), _model_tree_selected_get),
      EO_OP_FUNC(ELM_OBJ_MODEL_TREE_CONST_ID(ELM_OBJ_MODEL_TREE_CONST_SUB_ID_RELEASE), _model_tree_release),
      EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_descs);
}

static const Eo_Op_Description model_tree_const_op_descs[] = {
   EO_OP_DESCRIPTION(ELM_OBJ_MODEL_TREE_CONST_SUB_ID_SELECT, "Select the node pointed by path."),
   EO_OP_DESCRIPTION(ELM_OBJ_MODEL_TREE_CONST_SUB_ID_VALUE_GET, "Get the value of the node pointed by path."),
   EO_OP_DESCRIPTION(ELM_OBJ_MODEL_TREE_CONST_SUB_ID_CHILDREN_GET, "Get the children of the node pointed by path."),
   EO_OP_DESCRIPTION(ELM_OBJ_MODEL_TREE_CONST_SUB_ID_SELECTED_GET, "Get the path of the selected node."),
   EO_OP_DESCRIPTION(ELM_OBJ_MODEL_TREE_CONST_SUB_ID_RELEASE, "Release the sub-tree pointed by path and all its children. Release does not delete the nodes, it just frees their nodes."),
   EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Event_Description *model_tree_const_event_descs[] = {
     ELM_MODEL_TREE_CONST_SELECT_EVT,
     NULL
};

static Eo_Class_Description model_tree_const_class_descs = {
   EO_VERSION,
   "Model Tree",
   EO_CLASS_TYPE_REGULAR_NO_INSTANT,
   EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_MODEL_TREE_CONST_BASE_ID, model_tree_const_op_descs, ELM_OBJ_MODEL_TREE_CONST_SUB_ID_LAST),
   model_tree_const_event_descs, // event_descs
   sizeof(Elm_Model_Tree),
   _model_tree_const_class_constructor,
   NULL
};

EO_DEFINE_CLASS(elm_obj_model_tree_const_class_get, &model_tree_const_class_descs, EO_BASE_CLASS, NULL);


/**
 * @see elm_model_tree_mutable.h
 */
EAPI Eo_Op ELM_MODEL_TREE_BASE_ID = 0;

#define MY_MODEL_TREE_CLASS ELM_MODEL_TREE_CLASS

EAPI const Eo_Event_Description _TREE_CHILD_APPEND_EVT =
   EO_EVENT_DESCRIPTION("child,append","Append a new child cointaining value to the \
                        list of children of the node pointed by path.");

EAPI const Eo_Event_Description _TREE_DELETE_EVT =
   EO_EVENT_DESCRIPTION("tree,delete","Delete the sub-tree pointed path and all its children.");

EAPI const Eo_Event_Description _TREE_VALUE_SET_EVT =
   EO_EVENT_DESCRIPTION("value,set","Set value to the node pointed by path");

/*
 * Mutable Class definition
 */
//TODO/FIXME/XXX
static void
_model_tree_child_append(Eo *obj, void *class_data, va_list *list)
{
   Elm_Model_Tree_Node *node, *parent;
   Elm_Model_Tree *model = (Elm_Model_Tree *)eo_data_scope_get(obj, ELM_OBJ_MODEL_TREE_CONST_CLASS);

   void *unused = va_arg(*list, void *); //TODO: check this
   Elm_Model_Tree_Path *path = va_arg(*list, Elm_Model_Tree_Path *);
   Eina_Value *value = va_arg(*list, Eina_Value *);
   Elm_Model_Tree_Path **ret = va_arg(*list, Elm_Model_Tree_Path **);

   EINA_SAFETY_ON_NULL_RETURN(model);
   eina_lock_take(&model->lock);
   parent = _tree_node_find(model->root, path);
   EINA_SAFETY_ON_NULL_GOTO(parent, exit_err);
   node = _tree_node_append(value, parent);
   EINA_SAFETY_ON_NULL_GOTO(parent, exit_err);
   EINA_SAFETY_ON_NULL_GOTO(node, exit_err);
   *ret = _tree_node_path(node);
   eina_lock_release(&model->lock);

   eo_do(obj, eo_event_callback_call(TREE_CHILD_APPEND_EVT, *ret, NULL));

 exit_err:
   eina_lock_release(&model->lock);
}

static void
_model_tree_child_prepend(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED, va_list *list)
{
   Elm_Model_Tree_Node *node, *parent;
   Elm_Model_Tree *model = eo_data_scope_get(obj, MY_MODEL_TREE_CLASS);

   Elm_Model_Tree_Path *path = va_arg(*list, Elm_Model_Tree_Path *);
   Eina_Value *value = va_arg(*list, Eina_Value *);
   Elm_Model_Tree_Path **ret = va_arg(*list, Elm_Model_Tree_Path **);

   EINA_SAFETY_ON_NULL_RETURN(model);
   eina_lock_take(&model->lock);

   parent = _tree_node_find(model->root, path);
   EINA_SAFETY_ON_NULL_GOTO(parent, exit_err);
   node = _tree_node_prepend(value, parent);
   EINA_SAFETY_ON_NULL_GOTO(node, exit_err);
   *ret = _tree_node_path(node);
   eina_lock_release(&model->lock);

   eo_do(obj, eo_event_callback_call(TREE_CHILD_APPEND_EVT, ret, NULL));

   return;

exit_err:
   eina_lock_release(&model->lock);
}

static void
_model_tree_child_append_relative(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   return; // TODO implement
}

static void
_model_tree_child_prepend_relative(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   return; // TODO implement
}

static void
_model_tree_delete(Eo *obj, void *class_data EINA_UNUSED, va_list *list)
{
   Elm_Model_Tree *model = eo_data_scope_get(obj, ELM_OBJ_MODEL_TREE_CONST_CLASS);
   Elm_Model_Tree_Path *path = va_arg(*list, Elm_Model_Tree_Path *);

   EINA_SAFETY_ON_NULL_RETURN(path);
   EINA_SAFETY_ON_NULL_RETURN(model);
   eo_do(obj, elm_model_tree_release(path));

   eo_do(obj, eo_event_callback_call(TREE_DELETE_EVT, path, NULL));
}

static void
_model_tree_value_set(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED, va_list *list)
{
   Elm_Model_Tree_Node *node;
   Elm_Model_Tree *model = eo_data_scope_get(obj, ELM_OBJ_MODEL_TREE_CONST_CLASS);
   (void)va_arg(*list, void *);
   Elm_Model_Tree_Path *path = va_arg(*list, Elm_Model_Tree_Path *);
   Eina_Value *value = va_arg(*list, Eina_Value *);

   if(path == NULL) return;
   EINA_SAFETY_ON_NULL_RETURN(model);
   eina_lock_take(&model->lock);
   node = _tree_node_find(model->root, path);
   _tree_node_value_set(node, value);
   eina_lock_release(&model->lock);

   //TODO/FIXME <ccarvalho>
   //eo2_do(obj, _model_tree_value_set_callback_call(path, value));
   //eo_do(obj, eo_event_callback_call(TREE_VALUE_SET_EVT, "x", NULL));
}

void
_model_tree_value_set_evt(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   //TODO: Implement 
}

static void
_mutable_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_descs[] = {
      EO_OP_FUNC(ELM_OBJ_MODEL_TREE_ID(ELM_OBJ_MUTABLE_SUB_ID_CHILD_APPEND), _model_tree_child_append),
      EO_OP_FUNC(ELM_OBJ_MODEL_TREE_ID(ELM_OBJ_MUTABLE_SUB_ID_CHILD_PREPEND), _model_tree_child_prepend),
      EO_OP_FUNC(ELM_OBJ_MODEL_TREE_ID(ELM_OBJ_MUTABLE_SUB_ID_CHILD_APPEND_RELATIVE), _model_tree_child_append_relative),
      EO_OP_FUNC(ELM_OBJ_MODEL_TREE_ID(ELM_OBJ_MUTABLE_SUB_ID_CHILD_PREPEND_RELATIVE), _model_tree_child_prepend_relative),
      EO_OP_FUNC(ELM_OBJ_MODEL_TREE_ID(ELM_OBJ_MUTABLE_SUB_ID_TREE_DELETE), _model_tree_delete),
      EO_OP_FUNC(ELM_OBJ_MODEL_TREE_ID(ELM_OBJ_MUTABLE_SUB_ID_TREE_VALUE_SET), _model_tree_value_set),
      EO_OP_FUNC(ELM_OBJ_MODEL_TREE_ID(ELM_OBJ_MUTABLE_SUB_ID_TREE_CHILD_APPEND_EVT), _model_tree_value_set_evt),
      EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_descs);
}


static const Eo_Op_Description mutable_op_descs[] = {
   EO_OP_DESCRIPTION(ELM_OBJ_MUTABLE_SUB_ID_CHILD_APPEND, "Append new child"),
   EO_OP_DESCRIPTION(ELM_OBJ_MUTABLE_SUB_ID_CHILD_PREPEND, "Prepend new child"),
   EO_OP_DESCRIPTION(ELM_OBJ_MUTABLE_SUB_ID_CHILD_APPEND_RELATIVE, "Append as a siblind node"),
   EO_OP_DESCRIPTION(ELM_OBJ_MUTABLE_SUB_ID_CHILD_PREPEND_RELATIVE, "Prepend as a sibling node"),
   EO_OP_DESCRIPTION(ELM_OBJ_MUTABLE_SUB_ID_TREE_DELETE, "Delete node"),
   EO_OP_DESCRIPTION(ELM_OBJ_MUTABLE_SUB_ID_TREE_VALUE_SET, "Set value to the node"),
   EO_OP_DESCRIPTION(ELM_OBJ_MUTABLE_SUB_ID_TREE_CHILD_APPEND_EVT, "Append new child event."),
   EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Event_Description *mutable_event_descs[] = {
   TREE_CHILD_APPEND_EVT,
   TREE_DELETE_EVT,
   TREE_VALUE_SET_EVT,
   NULL
};

static Eo_Class_Description mutable_class_descs = {
   EO_VERSION,
   "Model Tree",
   EO_CLASS_TYPE_REGULAR,
   EO_CLASS_DESCRIPTION_OPS(&ELM_MODEL_TREE_BASE_ID, mutable_op_descs, ELM_OBJ_MUTABLE_SUB_ID_LAST),
   mutable_event_descs, // event_descs
   0,
   _mutable_class_constructor,
   NULL
};

//TODO/FIXME/XXX: inherit from MY_CLASS
//EO_DEFINE_CLASS(elm_obj_tree_mutable_class_get, &class_descs, EO_BASE_CLASS, NULL);
EO_DEFINE_CLASS(elm_obj_model_tree_class_get, &mutable_class_descs, ELM_OBJ_MODEL_TREE_CONST_CLASS, NULL);

//FIXME  ccarvalho
//EO3_DEFINE_CLASS(MY_MODEL_TREE_CLASS, ((MY_CLASS)), NULL)
