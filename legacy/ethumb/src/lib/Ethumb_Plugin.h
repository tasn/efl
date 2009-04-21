#ifndef _ETHUMB_PLUGIN_H_
#define _ETHUMB_PLUGIN_H_

#include <Ethumb.h>
#include <Eina.h>
#include <Evas.h>

typedef struct _Ethumb_Plugin Ethumb_Plugin;

struct _Ethumb_Plugin
{
   const char **extensions;
   int (*generate_thumb)(Ethumb_File *);
   void (*shutdown)(Ethumb_Plugin *);
};

void ethumb_calculate_aspect(Ethumb *e, int iw, int ih, int *w, int *h);
void ethumb_calculate_fill(Ethumb *e, int iw, int ih, int *fx, int *fy, int *fw, int *fh);
int ethumb_plugin_image_resize(Ethumb_File *ef, int w, int h);
int ethumb_image_save(Ethumb_File *ef);
void ethumb_finished_callback_call(Ethumb_File *ef);

#endif /* _ETHUMB_PLUGIN_H_ */
