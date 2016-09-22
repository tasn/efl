#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static const struct {
   Evas_Image_Orient orient;
   const char *name;
} images_orient[] = {
  { EVAS_IMAGE_ORIENT_NONE, "None" },
  { EVAS_IMAGE_ORIENT_90, "Rotate 90" },
  { EVAS_IMAGE_ORIENT_180, "Rotate 180" },
  { EVAS_IMAGE_ORIENT_270, "Rotate 270" },
  { EVAS_IMAGE_FLIP_HORIZONTAL, "Horizontal Flip" },
  { EVAS_IMAGE_FLIP_VERTICAL, "Vertical Flip" },
  { EVAS_IMAGE_FLIP_TRANSPOSE, "Transpose" },
  { EVAS_IMAGE_FLIP_TRANSVERSE, "Transverse" },
  { 0, NULL }
};

static void
my_im_ch(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   Evas_Object *im = evas_object_data_get(win, "im");
   Evas_Object *rdg = evas_object_data_get(win, "rdg");
   Elm_Image_Orient v = elm_radio_value_get(rdg);

   elm_image_orient_set(im, v);
   fprintf(stderr, "Set %i and got %i\n",
           v, elm_image_orient_get(im));
}

void
test_image(void *data EINA_UNUSED, Evas_Object *obj  EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *im, *rd, *rdg = NULL;
   int i;

   win = elm_win_util_standard_add("image test", "Image Test");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   im = elm_image_add(win);
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   elm_image_file_set(im, buf, NULL);
   evas_object_size_hint_weight_set(im, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(im, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, im);
   evas_object_show(im);

   evas_object_data_set(win, "im", im);

   for (i = 0; images_orient[i].name; ++i)
     {
        rd = elm_radio_add(win);
        evas_object_size_hint_align_set(rd, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, 0.0);
        elm_radio_state_value_set(rd, images_orient[i].orient);
        elm_object_text_set(rd, images_orient[i].name);
        elm_box_pack_end(box, rd);
        evas_object_show(rd);
        evas_object_smart_callback_add(rd, "changed", my_im_ch, win);
        if (!rdg)
          {
             rdg = rd;
             evas_object_data_set(win, "rdg", rdg);
          }
        else
          {
             elm_radio_group_add(rd, rdg);
          }
     }

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

static void
_download_start_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data, *txt;
   const char *url = NULL;
   char buf[4096] = {0};

   txt = evas_object_data_get(win, "txt");
   elm_image_file_get(txt, &url, NULL);
   snprintf(buf, sizeof(buf) - 1, "Remote image download started:\n%s", url);
   elm_object_text_set(txt, buf);
   printf("%s\n", buf);
   fflush(stdout);
}

static void
_download_progress_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Image_Progress *p = event_info;
   Evas_Object *win = data, *txt;
   char buf[4096] = {0};

   txt = evas_object_data_get(win, "txt");
   snprintf(buf, sizeof(buf) - 1, "Remote image download progress %.2f/%.2f.", p->now, p->total);
   elm_object_text_set(txt, buf);
   printf("%s\n", buf);
   fflush(stdout);
}

static void
_download_done_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data, *txt;
   char buf[4096] = {0};

   txt = evas_object_data_get(win, "txt");
   snprintf(buf, sizeof(buf) - 1, "Remote image download done.");
   elm_object_text_set(txt, buf);
   printf("%s\n", buf);
   fflush(stdout);

   evas_object_hide(txt);
}

static void
_download_error_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data, *txt;
   char buf[4096] = {0};

   txt = evas_object_data_get(win, "txt");
   snprintf(buf, sizeof(buf) - 1, "Remote image download failed.");
   elm_object_text_set(txt, buf);
   printf("%s\n", buf);
   fflush(stdout);

   evas_object_show(txt);
}

static void
_url_activate_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data, *txt, *im;
   const char *url;

   im = evas_object_data_get(win, "im");
   txt = evas_object_data_get(win, "txt");

   url = elm_object_text_get(obj);
   elm_image_file_set(im, url, NULL);

   evas_object_show(txt);
}

void
test_remote_image(void *data EINA_UNUSED, Evas_Object *obj  EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *im, *rd, *rdg = NULL, *box2, *o, *tbl, *txt;
   int i;

   win = elm_win_util_standard_add("image test", "Image Test");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   tbl = o = elm_table_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);

   txt = o = elm_label_add(box);
   elm_label_line_wrap_set(o, 1);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tbl, o, 0, 0, 1, 1);
   evas_object_data_set(win, "txt", o);
   evas_object_hide(o);

   im = o = elm_image_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_data_set(win, "im", o);
   elm_table_pack(tbl, o, 0, 0, 1, 1);
   evas_object_show(o);

   elm_box_pack_end(box, tbl);
   evas_object_show(tbl);

   evas_object_smart_callback_add(im, "download,start", _download_start_cb, win);
   evas_object_smart_callback_add(im, "download,progress", _download_progress_cb, win);
   evas_object_smart_callback_add(im, "download,done", _download_done_cb, win);
   evas_object_smart_callback_add(im, "download,error", _download_error_cb, win);

   for (i = 0; images_orient[i].name; ++i)
     {
        rd = elm_radio_add(win);
        evas_object_size_hint_align_set(rd, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, 0.0);
        elm_radio_state_value_set(rd, images_orient[i].orient);
        elm_object_text_set(rd, images_orient[i].name);
        elm_box_pack_end(box, rd);
        evas_object_show(rd);
        evas_object_smart_callback_add(rd, "changed", my_im_ch, win);
        if (!rdg)
          {
             rdg = rd;
             evas_object_data_set(win, "rdg", rdg);
          }
        else
          {
             elm_radio_group_add(rd, rdg);
          }
     }

   box2 = o = elm_box_add(box);
   elm_box_horizontal_set(o, 1);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);

   o = elm_label_add(box2);
   elm_object_text_set(o, "URL:");
   elm_box_pack_end(box2, o);
   evas_object_show(o);

   o = elm_entry_add(box2);
   elm_entry_scrollable_set(o, 1);
   elm_entry_single_line_set(o, 1);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(o, "http://41.media.tumblr.com/29f1ecd4f98aaff73fb21f479b450d4c/tumblr_mqsxdciQmB1rrju89o1_1280.jpg");
   evas_object_smart_callback_add(o, "activated", _url_activate_cb, win);
   elm_box_pack_end(box2, o);
   evas_object_show(o);

   elm_box_pack_end(box, box2);
   evas_object_show(box2);

   // set file now
   _url_activate_cb(win, o, NULL);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

static void
_img_clicked_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Elm_Transit *trans;
   static int degree = 0;

   fprintf(stderr, "%p - clicked\n", obj);

   trans = elm_transit_add();
   elm_transit_object_add(trans, data);
   if (degree == 0)
     {
        elm_transit_effect_rotation_add(trans, 0, 180);
        degree = 180;
     }
   else
     {
        elm_transit_effect_rotation_add(trans, 180, 360);
        degree = 0;
     }
   elm_transit_duration_set(trans, 3.0);
   elm_transit_objects_final_state_keep_set(trans, EINA_TRUE);
   elm_transit_go(trans);
}

void
test_click_image(void *data EINA_UNUSED, Evas_Object *obj  EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *im, *label;

   win = elm_win_util_standard_add("image test", "Image Test");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   im = elm_image_add(win);
   elm_object_focus_allow_set(im, EINA_TRUE);
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   elm_image_file_set(im, buf, NULL);
   evas_object_size_hint_weight_set(im, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(im, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(im, "clicked", _img_clicked_cb, im);
   elm_box_pack_end(box, im);
   evas_object_show(im);
   elm_object_focus_set(im, EINA_TRUE);

   label = elm_label_add(win);
   elm_object_text_set(label, "<b>Press Return/Space/KP_Return key on image to transit.</b>");
   evas_object_size_hint_weight_set(label, 0.0, 0.0);
   evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, label);
   evas_object_show(label);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
