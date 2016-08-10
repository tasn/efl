#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "constructors_mixin.h"
#include "constructors_simple7.h"
#include "constructors_simple2.h"

#include "../eunit_tests.h"

#define MY_CLASS SIMPLE7_CLASS

static Eo *
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   /* FIXME: Actually test it. */
   return efl_constructor(eo_super(obj, MY_CLASS));
}

static Efl_Op_Description op_descs [] = {
     EO_OP_FUNC_OVERRIDE(efl_constructor, _constructor),
};

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Simple7",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple7_class_get, &class_desc, SIMPLE2_CLASS, NULL);

