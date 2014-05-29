//Compile with:
//gcc -g image_example_01.c -o image_example_01 `pkg-config --cflags --libs elementary`

#include <Elementary.h>

int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *image, *fs_bt, *box;
   char buf[PATH_MAX];

   elm_app_info_set(elm_main, "elementary", "images/an.c");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("image", "Image");
   elm_win_autodel_set(win, EINA_TRUE);

   snprintf(buf, sizeof(buf), "%s/images/an.c", elm_app_data_dir_get());
   printf("\n%s\n", buf);
   image = elm_image_add(win);
   if (!elm_image_file_set(image, buf, NULL))
     {
        printf("error: could not load image \"%s\"\n", buf);
        return -1;
     }

   elm_image_no_scale_set(image, EINA_TRUE);
   elm_image_resizable_set(image, EINA_FALSE, EINA_TRUE);
   elm_image_smooth_set(image, EINA_FALSE);
   elm_image_orient_set(image, ELM_IMAGE_FLIP_HORIZONTAL);
   elm_image_aspect_fixed_set(image, EINA_TRUE);
   elm_image_fill_outside_set(image, EINA_TRUE);
   elm_image_editable_set(image, EINA_TRUE);

   evas_object_size_hint_weight_set(image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, image);
   

   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); 
   fs_bt = elm_fileselector_button_add(box);
   elm_win_resize_object_add(win, box);
   
   elm_fileselector_button_path_set(fs_bt, "/home");
   elm_object_text_set(fs_bt, "Select a file");
   //elm_object_part_content_set(fs_bt, "icon", ic);
   elm_box_pack_end(box, fs_bt);
   //evas_object_size_hint_min_set(fs_bt, 100, 30);
   //elm_box_pack_start(box, image);
   elm_fileselector_button_inwin_mode_set(fs_bt, EINA_TRUE);
   evas_object_show(image);
   evas_object_show(fs_bt);
   evas_object_show(box);
   

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()
