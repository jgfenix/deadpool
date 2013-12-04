
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#include "elm_priv.h"
#include "elm_model_tree.h"
#include <assert.h>

struct _Elm_Model_Tree_Path
{
   unsigned int depth;
   unsigned int *indices;
};

Elm_Model_Tree_Path*
elm_model_tree_path_new()
{
   Elm_Model_Tree_Path *path = malloc(sizeof(Elm_Model_Tree_Path));
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   return path;
}

void
elm_model_tree_path_free(Elm_Model_Tree_Path *path)
{
   assert(path);
   free(path->indices);
   free(path);
}

Elm_Model_Tree_Path*
elm_model_tree_path_new_from_string(const char *str)
{
   char *ptr;
   Elm_Model_Tree_Path *path = elm_model_tree_path_new();
   long n;

   while( (n = strtol(str, &ptr, 10)) >= 0)
     {
        elm_model_tree_path_append_index(path, n);
        if(*ptr == '\0') break;
        EINA_SAFETY_ON_FALSE_GOTO(ptr != str && *ptr == ',', err_exit);
        str = ptr + 1;
     }
   EINA_SAFETY_ON_FALSE_GOTO(n >= 0, err_exit);

   return path;
   
 err_exit:
   elm_model_tree_path_free(path);
   return NULL;
}

void
elm_model_tree_path_append_index(Elm_Model_Tree_Path *path, unsigned int index)
{
   unsigned int *indices;

   assert(path);
   indices = malloc((path->depth + 1) * sizeof(unsigned int*));
   assert(indices);
   memcpy(indices, path->indices, sizeof(unsigned int) * path->depth);
   indices[path->depth] = index;
   free(path->indices);
   path->indices = indices;
   path->depth += 1;
}

void
elm_model_tree_path_prepend_index(Elm_Model_Tree_Path *path, unsigned int index)
{
   unsigned int*indices;

   assert(path);
   indices = malloc((path->depth + 1) * sizeof(unsigned int*));
   assert(indices);
   indices[0] = index;
   memcpy(indices + 1, path->indices, sizeof(unsigned int) * path->depth);
   free(path->indices);
   path->indices = indices;
   path->depth += 1;
}

char*
elm_model_tree_path_to_string(Elm_Model_Tree_Path *path)
{
   char *str;
   unsigned int i, n, r, res, *index;
   
   EINA_SAFETY_ON_FALSE_RETURN_VAL(path, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(path->depth, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(path->indices, NULL);

   for(n = path->depth, i = 0; i < path->depth; i++, n++)
      for(res = path->indices[i]; (res /= 10) > 0; n++)
         ;

   str = calloc(sizeof(char), n);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(str, NULL);

   index = path->indices;
   i = snprintf(str, n, "%u", *(index++));
   EINA_SAFETY_ON_FALSE_GOTO(i, err_exit);
   while(i < n)
     {
        r = snprintf(str + i, n - i, ",%u", *(index++));
        EINA_SAFETY_ON_FALSE_GOTO(r, err_exit);
        i += r;
     }
   return str;

 err_exit:
   free(str);
   return NULL;
}

unsigned int
elm_model_tree_path_get_depth(Elm_Model_Tree_Path *path)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(path, (unsigned int)-1);
   return path->depth;
}

unsigned int*
elm_model_tree_path_get_indices(Elm_Model_Tree_Path *path)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(path, NULL);
   return path->indices;
}

unsigned int
elm_model_tree_path_get_index(Elm_Model_Tree_Path *path, unsigned int depth)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(path, (unsigned int)-1);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(depth <= path->depth, (unsigned int)-1);   
   return path->indices[depth];
}


EO3_DEFINE_INTERFACE(ELM_MODEL_TREE_CONST_INTERFACE, ((EO3_NO_BASE_CLASS)))

EO3_DEFINE_INTERFACE(ELM_MODEL_TREE_INTERFACE, ((ELM_MODEL_TREE_CONST_INTERFACE)))


