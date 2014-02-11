#include <Eina.h>
#include <string.h>

#include "Eolian.h"
#include "eo1_generator.h"
#include "common_funcs.h"

static const char
tmpl_eo_src_begin[] = "\
EAPI Eo_Op @#OBJCLASS_BASE_ID = EO_NOOP;\n\
\n\
@#list_events\n\
\n\
";

static const char
tmpl_eo_src_end[] = "\
@#list_ctors_body\
\n\
static void\n\
_@#class_class_constructor(Eo_Class *klass)\n\
{\n\
   const Eo_Op_Func_Description func_desc[] = {@#list_func\n\
        EO_OP_FUNC_SENTINEL\n\
   };\n\
   eo_class_funcs_set(klass, func_desc);\n\
   _user_@#class_class_constructor(klass);\n\
}\n\
\n\
static const Eo_Op_Description @#class_op_desc[] = {@#list_op\n\
     EO_OP_DESCRIPTION_SENTINEL\n\
};\n\
\n\
static const Eo_Event_Description *@#class_event_desc[] = {@#list_evdesc\n\
    NULL\n\
};\n\
\n\
static const Eo_Class_Description @#class_class_desc = {\n\
     EO_VERSION,\n\
     \"@#Class\",\n\
     EO_CLASS_TYPE_REGULAR,\n\
     EO_CLASS_DESCRIPTION_OPS(&@#OBJCLASS_BASE_ID, @#class_op_desc, @#OBJCLASS_SUB_ID_LAST),\n\
     @#class_event_desc,\n\
     sizeof(@#Class_Data),\n\
     _@#class_class_constructor,\n\
     NULL\n\
};\n\
\n\
EO_DEFINE_CLASS(@#objclass_class_get, &@#class_class_desc, @#list_inheritNULL);\
";

static const char
tmpl_eo_op_desc[] = "\n     EO_OP_DESCRIPTION(@#OBJCLASS_SUB_ID_@#FUNC, \"@#desc\"),";

static const char
tmpl_eo_func_desc[] = "\n        EO_OP_FUNC(@#OBJCLASS_ID(@#OBJCLASS_SUB_ID_@#FUNC), _eo_obj_@#class_@#func),";

static const char
tmpl_eobase_func_desc[] = "\n        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_@#FUNC), _eo_obj_@#class_@#func),";

static const char
tmpl_eo_header[] = "\
#define @#OBJCLASS_CLASS @#objclass_class_get()\n\
\n\
const Eo_Class *@#objclass_class_get(void) EINA_CONST;\n\
\n\
extern EAPI Eo_Op @#OBJCLASS_BASE_ID;\n\
\n\
enum\n\
{@#list_subid\n\
   @#OBJCLASS_SUB_ID_LAST\n\
};\n\
\n\
#define @#OBJCLASS_ID(sub_id) (@#OBJCLASS_BASE_ID + sub_id)\n\
";

static const char
tmpl_eo_subid[] = "\n   @#OBJCLASS_SUB_ID_@#FUNC,";

static const char
tmpl_eo_subid_apnd[] = "   @#OBJCLASS_SUB_ID_@#FUNC,\n";

static const char
tmpl_eo_funcdef[] = "\n\
/**\n\
 * @def @#objclass_@#func\n\
 *\n\
@#desc\n\
 *\n\
@#list_desc_param\
 *\n\
 */\n\
#define @#eoprefix_@#func(@#list_param) @#OBJCLASS_ID(@#OBJCLASS_SUB_ID_@#FUNC) @#list_typecheck\n\
";

static const char
tmpl_eo_pardesc[] =" * @param[%s] %s\n";

static const char
tmpl_eobind_body[] ="\
\n\
@#ret_type _@#class_@#func(Eo *obj@#full_params);\n\n\
static void\n\
_eo_obj_@#class_@#func(Eo *obj, void *_pd EINA_UNUSED, va_list *list@#list_unused)\n\
{\n\
@#list_vars\
   @#ret_param_@#class_@#func(obj@#list_params);\n\
}\n\
";

char *
_first_line_get(const char *str)
{
   Eina_Strbuf *ret = eina_strbuf_new();
   if (str)
     {
        const char *p = strchr(str, '\n');
        size_t offs = (p) ? (size_t)(p - str) : strlen(str);
        eina_strbuf_append_n(ret, str, offs);
     }
   return eina_strbuf_string_steal(ret);
}

Eina_Bool
eo1_enum_append(const char *classname, const char *funcname, Eina_Strbuf *str)
{
   _template_fill(str, tmpl_eo_subid_apnd, classname, funcname, EINA_FALSE);
   return EINA_TRUE;
}

Eina_Bool
eo1_fundef_generate(const char *classname, Eolian_Function func, Eolian_Function_Type ftype, Eina_Strbuf *functext)
{
   const char *str_dir[] = {"in", "out", "inout"};
   const Eina_List *l;
   void *data;
   char funcname[0xFF];
   char descname[0xFF];

   char *fsuffix = "";
   if (ftype == GET) fsuffix = "_get";
   if (ftype == SET) fsuffix = "_set";

   sprintf (funcname, "%s%s", eolian_function_name_get(func), fsuffix);
   sprintf (descname, "comment%s", fsuffix);
   const char *funcdesc = eolian_function_description_get(func, descname);

   Eina_Strbuf *str_func = eina_strbuf_new();
   _template_fill(str_func, tmpl_eo_funcdef, classname, funcname, EINA_TRUE);

   Eina_Strbuf *linedesc = eina_strbuf_new();
   eina_strbuf_append(linedesc, funcdesc ? funcdesc : "");
   if (eina_strbuf_length_get(linedesc))
     {
        eina_strbuf_replace_all(linedesc, "\n", "\n * ");
        eina_strbuf_prepend(linedesc," * ");
     }

   eina_strbuf_replace_all(str_func, "@#desc", eina_strbuf_string_get(linedesc));
   eina_strbuf_free(linedesc);

   Eina_Strbuf *str_par = eina_strbuf_new();
   Eina_Strbuf *str_pardesc = eina_strbuf_new();
   Eina_Strbuf *str_typecheck = eina_strbuf_new();

   const char* rettype = eolian_function_return_type_get(func, ftype);
   if (rettype && strcmp(rettype, "void"))
     {
        eina_strbuf_append_printf(str_pardesc, tmpl_eo_pardesc, "out", "ret");
        eina_strbuf_append(str_par, "ret");
        eina_strbuf_append_printf(str_typecheck, ", EO_TYPECHECK(%s*, ret)", rettype);
     }

   EINA_LIST_FOREACH(eolian_parameters_list_get(func), l, data)
     {
        const char *pname;
        const char *ptype;
        Eolian_Parameter_Dir pdir;
        eolian_parameter_information_get((Eolian_Function_Parameter)data, &pdir, &ptype, &pname, NULL);
        if (ftype == GET) pdir = EOLIAN_OUT_PARAM;
        if (ftype == SET) pdir = EOLIAN_IN_PARAM;
        char *umpr = (pdir == EOLIAN_IN_PARAM) ? "" : "*";

        const char *dir_str = str_dir[(int)pdir];

        eina_strbuf_append_printf(str_pardesc, tmpl_eo_pardesc, dir_str, pname);

        if (eina_strbuf_length_get(str_par)) eina_strbuf_append(str_par, ", ");
        eina_strbuf_append(str_par, pname);

        eina_strbuf_append_printf(str_typecheck, ", EO_TYPECHECK(%s%s, %s)", ptype, umpr, pname);
     }

   eina_strbuf_replace_all(str_func, "@#list_param", eina_strbuf_string_get(str_par));
   eina_strbuf_replace_all(str_func, "@#list_desc_param", eina_strbuf_string_get(str_pardesc));
   eina_strbuf_replace_all(str_func, "@#list_typecheck", eina_strbuf_string_get(str_typecheck));

   eina_strbuf_free(str_par);
   eina_strbuf_free(str_pardesc);
   eina_strbuf_free(str_typecheck);

   eina_strbuf_append(functext, eina_strbuf_string_get(str_func));
   eina_strbuf_free(str_func);

   return EINA_TRUE;
}

Eina_Bool
eo1_header_generate(const char *classname, Eina_Strbuf *buf)
{
   const Eolian_Function_Type ftype_order[] = {CONSTRUCTOR, PROPERTY_FUNC, METHOD_FUNC};
   const Eina_List *l;
   void *data;
   char tmpstr[0x1FF];

   if (!eolian_class_exists(classname))
     {
        printf ("Class \"%s\" not found in database\n", classname);
        return EINA_FALSE;
     }

   Eina_Strbuf * str_hdr = eina_strbuf_new();
   _template_fill(str_hdr, tmpl_eo_header, classname, "", EINA_TRUE);

   Eina_Strbuf *str_subid = eina_strbuf_new();
   Eina_Strbuf *str_ev = eina_strbuf_new();
   Eina_Strbuf *tmpbuf = eina_strbuf_new();

   Eolian_Event event;
   EINA_LIST_FOREACH(eolian_class_events_list_get(classname), l, event)
     {
        const char *evname;
        const char *evdesc;
        eolian_class_event_information_get(event, &evname, &evdesc);

        eina_strbuf_reset(tmpbuf);
        eina_strbuf_append(tmpbuf, evdesc);
        eina_strbuf_replace_all(tmpbuf, "\n", "\n * ");
        eina_strbuf_prepend(tmpbuf," * ");
        eina_strbuf_append_printf(str_ev, "\n/**\n%s\n */\n", eina_strbuf_string_get(tmpbuf));

        _template_fill(tmpbuf, "@#CLASS_@#FUNC", classname, evname, EINA_TRUE);
        eina_strbuf_replace_all(tmpbuf, ",", "_");
        const char* s = eina_strbuf_string_get(tmpbuf);
        eina_strbuf_append_printf(str_ev, "#define %s (&(_%s))\n", s, s);
     }

   int i;
   for (i = 0; i < 3; i++)
      EINA_LIST_FOREACH(eolian_class_functions_list_get(classname, ftype_order[i]), l, data)
        {
           const Eolian_Function_Type ftype = eolian_function_type_get((Eolian_Function)data);
           const char *funcname = eolian_function_name_get((Eolian_Function)data);
           Eina_Bool prop_read = (ftype == PROPERTY_FUNC || ftype == GET ) ? EINA_TRUE : EINA_FALSE ;
           Eina_Bool prop_write = (ftype == PROPERTY_FUNC || ftype == SET ) ? EINA_TRUE : EINA_FALSE ;

           if (!prop_read && !prop_write)
             {
                _template_fill(str_subid, tmpl_eo_subid, classname, funcname, EINA_FALSE);
                eo1_fundef_generate(classname, (Eolian_Function)data, UNRESOLVED, str_hdr);
             }
           if (prop_read)
             {
                sprintf(tmpstr, "%s_get", funcname);
                _template_fill(str_subid, tmpl_eo_subid, classname, tmpstr, EINA_FALSE);
                eo1_fundef_generate(classname, (Eolian_Function)data, GET, str_hdr);
             }
           if (prop_write)
             {
                sprintf(tmpstr, "%s_set", funcname);
                _template_fill(str_subid, tmpl_eo_subid, classname, tmpstr, EINA_FALSE);
                eo1_fundef_generate(classname, (Eolian_Function)data, SET, str_hdr);
             }
        }

   eina_strbuf_replace_all(str_hdr, "@#list_subid", eina_strbuf_string_get(str_subid));
   eina_strbuf_append(str_hdr, eina_strbuf_string_get(str_ev));

   eina_strbuf_free(str_subid);
   eina_strbuf_free(str_ev);
   eina_strbuf_free(tmpbuf);

   eina_strbuf_append(buf, eina_strbuf_string_get(str_hdr));
   return EINA_TRUE;
}

static const char*
_varg_upgr(const char *stype)
{
    if (!strcmp(stype, "Eina_Bool") ||
        !strcmp(stype, "char") ||
        !strcmp(stype, "short"))
      return "int";
    return stype;
}

Eina_Bool
eo1_bind_func_generate(const char *classname, Eolian_Function funcid, Eolian_Function_Type ftype, Eina_Strbuf *buf)
{
   const char *suffix = "";
   const char *umpr = NULL;
   Eina_Bool var_as_ret = EINA_FALSE;
   const char *rettype = NULL;
   const char *retname = NULL;
   Eina_Bool ret_const = EINA_FALSE;

   Eina_Strbuf *fbody = eina_strbuf_new();
   Eina_Strbuf *va_args = eina_strbuf_new();
   Eina_Strbuf *params = eina_strbuf_new(); /* only variables names */
   Eina_Strbuf *full_params = eina_strbuf_new(); /* variables types + names */

   rettype = eolian_function_return_type_get(funcid, ftype);
   if (rettype && !strcmp(rettype, "void")) rettype = NULL;
   retname = "ret";
   if (ftype == GET)
     {
        suffix = "_get";
        umpr = "*";
        if (!rettype)
          {
             const Eina_List *l = eolian_parameters_list_get(funcid);
             if (eina_list_count(l) == 1)
               {
                  void* data = eina_list_data_get(l);
                  eolian_parameter_information_get((Eolian_Function_Parameter)data, NULL, &rettype, &retname, NULL);
                  var_as_ret = EINA_TRUE;
                  ret_const = eolian_parameter_get_const_attribute_get(data);
               }
          }
     }
   if (ftype == SET)
     {
        suffix = "_set";
        umpr = "";
     }

   char tmpstr[0xFF];
   sprintf (tmpstr, "%s%s", eolian_function_name_get(funcid), suffix);
   _template_fill(fbody, tmpl_eobind_body, classname, tmpstr, EINA_FALSE);

   const Eina_List *l;
   void *data;

   if (!var_as_ret)
     {
        EINA_LIST_FOREACH(eolian_parameters_list_get(funcid), l, data)
          {
             const char *pname;
             const char *ptype;
             Eolian_Parameter_Dir pdir;
             eolian_parameter_information_get((Eolian_Function_Parameter)data, &pdir, &ptype, &pname, NULL);
             const char *ptrstr = (umpr) ? umpr : ( (pdir == EOLIAN_IN_PARAM) ? "" : " *" );
             Eina_Bool is_const = eolian_parameter_get_const_attribute_get(data);
             eina_strbuf_append_printf(va_args, "   %s%s %s%s = va_arg(*list, %s%s%s);\n",
                   ftype == GET && is_const?"const ":"", ptype, ptrstr, pname,
                   ftype == GET && is_const?"const ":"", (*ptrstr) ? ptype : _varg_upgr(ptype), ptrstr);
             eina_strbuf_append_printf(params, ", %s", pname);
             eina_strbuf_append_printf(full_params, ", %s%s%s %s",
                   ftype == GET && eolian_parameter_get_const_attribute_get(data)?"const ":"",
                   ptype, ptrstr, pname);
          }
     }

   if (rettype && strcmp(rettype, "void"))
     {
        eina_strbuf_append_printf(va_args, "   %s%s *%s = va_arg(*list, %s%s *);\n",
              ret_const?"const ":"",
              rettype, retname,
              ret_const?"const ":"",
              rettype);
        Eina_Strbuf *ret_param = eina_strbuf_new();
        eina_strbuf_append_printf(ret_param, "*%s = ", retname);
        eina_strbuf_replace_all(fbody, "@#ret_param", eina_strbuf_string_get(ret_param));
        sprintf(tmpstr, "%s%s", ret_const?"const ":"", rettype);
        eina_strbuf_replace_all(fbody, "@#ret_type", tmpstr);
        eina_strbuf_free(ret_param);
     }
   else
     {
        eina_strbuf_replace_all(fbody, "@#ret_param", "");
        eina_strbuf_replace_all(fbody, "@#ret_type", "void");
     }

   if (eina_list_count(eolian_parameters_list_get(funcid)) == 0)
     {
        eina_strbuf_replace_all(fbody, "@#list_unused", " EINA_UNUSED");
     }
   else
     {
        eina_strbuf_replace_all(fbody, "@#list_unused", "");
     }
   eina_strbuf_replace_all(fbody, "@#list_vars", eina_strbuf_string_get(va_args));
   eina_strbuf_replace_all(fbody, "@#full_params", eina_strbuf_string_get(full_params));
   eina_strbuf_replace_all(fbody, "@#list_params", eina_strbuf_string_get(params));
   eina_strbuf_append(buf, eina_strbuf_string_get(fbody));

   eina_strbuf_free(va_args);
   eina_strbuf_free(full_params);
   eina_strbuf_free(params);
   eina_strbuf_free(fbody);
   return EINA_TRUE;
}

Eina_Bool
eo1_eo_func_desc_generate(const char *classname, const char *funcname, Eina_Strbuf *buf)
{
   _template_fill(buf, tmpl_eo_func_desc, classname, funcname, EINA_TRUE);
   return EINA_TRUE;
}

Eina_Bool
eo1_eo_op_desc_generate(const char *classname, const char *funcname, Eina_Strbuf *buf)
{
   _template_fill(buf, tmpl_eo_op_desc, classname, funcname, EINA_TRUE);
   return EINA_TRUE;
}

Eina_Bool
eo1_source_beginning_generate(const char *classname, Eina_Strbuf *buf)
{
   const Eina_List *itr;

   Eina_Strbuf *tmpbuf = eina_strbuf_new();
   Eina_Strbuf *str_begin = eina_strbuf_new();
   Eina_Strbuf *str_ev = eina_strbuf_new();

   _template_fill(str_begin, tmpl_eo_src_begin, classname, "", EINA_TRUE);

   Eolian_Event event;
   EINA_LIST_FOREACH(eolian_class_events_list_get(classname), itr, event)
     {
        const char *evname;
        const char *evdesc;
        char *evdesc_line1;

        eolian_class_event_information_get(event, &evname, &evdesc);
        evdesc_line1 = _first_line_get(evdesc);
        _template_fill(str_ev, "@#CLASS_@#FUNC", classname, evname, EINA_TRUE);
        eina_strbuf_replace_all(str_ev, ",", "_");

        eina_strbuf_append_printf(tmpbuf,
                                  "EAPI const Eo_Event_Description _%s = EO_EVENT_DESCRIPTION(\"%s\", \"%s\");\n",
                                  eina_strbuf_string_get(str_ev), evname, evdesc_line1);
        free(evdesc_line1);
     }

   eina_strbuf_replace_all(str_begin, "@#list_events", eina_strbuf_string_get(tmpbuf));
   eina_strbuf_append(buf, eina_strbuf_string_get(str_begin));

   eina_strbuf_free(str_ev);
   eina_strbuf_free(tmpbuf);
   eina_strbuf_free(str_begin);
   return EINA_TRUE;
}

Eina_Bool
eo1_source_end_generate(const char *classname, Eina_Strbuf *buf)
{
   const Eina_List *itr;
   Eolian_Function fn;

   Eina_Strbuf *str_end = eina_strbuf_new();
   Eina_Strbuf *tmpbuf = eina_strbuf_new();
   Eina_Strbuf *str_op = eina_strbuf_new();
   Eina_Strbuf *str_func = eina_strbuf_new();
   Eina_Strbuf *str_bodyf = eina_strbuf_new();
   Eina_Strbuf *str_ev = eina_strbuf_new();

   _template_fill(str_end, tmpl_eo_src_end, classname, "", EINA_TRUE);

   // default constructor
   Eolian_Function ctor_fn = eolian_class_default_constructor_get(classname);
   if (ctor_fn)
     {
        _template_fill(str_func, tmpl_eobase_func_desc, classname, "constructor", EINA_FALSE);
        eo1_bind_func_generate(classname, ctor_fn, UNRESOLVED, str_bodyf);
     }
   // default destructor
   Eolian_Function dtor_fn = eolian_class_default_destructor_get(classname);
   if (dtor_fn)
     {
        _template_fill(str_func, tmpl_eobase_func_desc, classname, "destructor", EINA_FALSE);
        eo1_bind_func_generate(classname, dtor_fn, UNRESOLVED, str_bodyf);
     }

   //Implements - TODO one generate func def for all
   Eolian_Implement impl_desc;
   EINA_LIST_FOREACH(eolian_class_implements_list_get(classname), itr, impl_desc)
     {
        const char *funcname;
        const char *impl_class;
        Eolian_Function_Type ftype;

        eolian_implement_information_get(impl_desc, &impl_class, &funcname, &ftype);

        Eina_Strbuf *tmpl_impl = eina_strbuf_new();
        eina_strbuf_append(tmpl_impl, tmpl_eo_func_desc);

        char tbuff[0xFF];
        char *tp = tbuff;
        strcpy(tbuff, classname);
        eina_str_tolower(&tp);

        eina_strbuf_replace_all(tmpl_impl, "@#class", tbuff);
        const char *tmpl_impl_str = eina_strbuf_string_get(tmpl_impl);

        Eolian_Function in_meth = NULL;
        Eolian_Function in_prop = NULL;
        const Eina_List *itr2;
        Eolian_Function fnid;
        EINA_LIST_FOREACH(eolian_class_functions_list_get((char *)impl_class, CONSTRUCTOR), itr2, fnid)
          if (fnid && !strcmp(eolian_function_name_get(fnid), funcname)) in_meth = fnid;
        EINA_LIST_FOREACH(eolian_class_functions_list_get((char *)impl_class, METHOD_FUNC), itr2, fnid)
          if (fnid && !strcmp(eolian_function_name_get(fnid), funcname)) in_meth = fnid;
        EINA_LIST_FOREACH(eolian_class_functions_list_get((char *)impl_class, PROPERTY_FUNC), itr2, fnid)
          if (fnid && !strcmp(eolian_function_name_get(fnid), funcname)) in_prop = fnid;

        if (!in_meth && !in_prop)
          {
             printf ("Failed to generate implementation of %s:%s - missing form super class\n", impl_class, funcname);
             return EINA_FALSE;
          }

        if (in_meth)
          {
             _template_fill(str_func, tmpl_impl_str, impl_class, funcname, EINA_FALSE);
             eo1_bind_func_generate(classname, in_meth, UNRESOLVED, str_bodyf);
          }

        if (in_prop)
          {
             char tmpstr[0xFF];

             if ((ftype != GET) && (ftype != SET)) ftype = eolian_function_type_get(in_prop);

             Eina_Bool prop_read = ( ftype == SET ) ? EINA_FALSE : EINA_TRUE;
             Eina_Bool prop_write = ( ftype == GET ) ? EINA_FALSE : EINA_TRUE;

             if (prop_read)
               {
                  sprintf(tmpstr, "%s_get", funcname);
                  _template_fill(str_func, tmpl_impl_str, impl_class, tmpstr, EINA_FALSE);
                  eo1_bind_func_generate(classname, in_prop, GET, str_bodyf);
               }

             if (prop_write)
               {
                  sprintf(tmpstr, "%s_set", funcname);
                  _template_fill(str_func, tmpl_impl_str, impl_class, tmpstr, EINA_FALSE);
                 eo1_bind_func_generate(classname, in_prop, SET, str_bodyf);
               }
          }
          eina_strbuf_free(tmpl_impl);
     }

   //Constructors
   EINA_LIST_FOREACH(eolian_class_functions_list_get(classname, CONSTRUCTOR), itr, fn)
     {
        const char *funcname = eolian_function_name_get(fn);
        char *desc = _first_line_get(eolian_function_description_get(fn, "comment"));

        _template_fill(tmpbuf, tmpl_eo_op_desc, classname, funcname, EINA_TRUE);
        eina_strbuf_replace_all(tmpbuf, "@#desc", desc);
        free(desc);

        eina_strbuf_append(str_op, eina_strbuf_string_get(tmpbuf));

        _template_fill(str_func, tmpl_eo_func_desc, classname, funcname, EINA_FALSE);
        eo1_bind_func_generate(classname, fn, UNRESOLVED, str_bodyf);
     }

   //Properties
   EINA_LIST_FOREACH(eolian_class_functions_list_get(classname, PROPERTY_FUNC), itr, fn)
     {
        const char *funcname = eolian_function_name_get(fn);
        const Eolian_Function_Type ftype = eolian_function_type_get(fn);
        char tmpstr[0xFF];

        Eina_Bool prop_read = ( ftype == SET ) ? EINA_FALSE : EINA_TRUE;
        Eina_Bool prop_write = ( ftype == GET ) ? EINA_FALSE : EINA_TRUE;

        if (prop_read)
          {
             char *desc = _first_line_get(eolian_function_description_get(fn, "comment_get"));

             sprintf(tmpstr, "%s_get", funcname);
             eo1_eo_op_desc_generate(classname, tmpstr, tmpbuf);
             eina_strbuf_replace_all(tmpbuf, "@#desc", desc);
             free(desc);
             eina_strbuf_append(str_op, eina_strbuf_string_get(tmpbuf));

             eo1_eo_func_desc_generate(classname, tmpstr, tmpbuf);
             eina_strbuf_append(str_func, eina_strbuf_string_get(tmpbuf));
          }
        if (prop_write)
          {
             char *desc = _first_line_get(eolian_function_description_get(fn, "comment_set"));

             sprintf(tmpstr, "%s_set", funcname);
             eo1_eo_op_desc_generate(classname, tmpstr, tmpbuf);
             eina_strbuf_replace_all(tmpbuf, "@#desc", desc);
             eina_strbuf_append(str_op, eina_strbuf_string_get(tmpbuf));
             free(desc);

             eo1_eo_func_desc_generate(classname, tmpstr, tmpbuf);
             eina_strbuf_append(str_func, eina_strbuf_string_get(tmpbuf));
          }
     }

   //Methods
   EINA_LIST_FOREACH(eolian_class_functions_list_get(classname, METHOD_FUNC), itr, fn)
     {
        const char *funcname = eolian_function_name_get(fn);
        char *desc = _first_line_get(eolian_function_description_get(fn, "comment"));

        eo1_eo_op_desc_generate(classname, funcname, tmpbuf);
        eina_strbuf_replace_all(tmpbuf, "@#desc", desc);
        free(desc);
        eina_strbuf_append(str_op, eina_strbuf_string_get(tmpbuf));

        eo1_eo_func_desc_generate(classname, funcname, tmpbuf);
        eina_strbuf_append(str_func, eina_strbuf_string_get(tmpbuf));
     }

   Eolian_Event event;
   EINA_LIST_FOREACH(eolian_class_events_list_get(classname), itr, event)
     {
        const char *evname;

        eolian_class_event_information_get(event, &evname, NULL);
        _template_fill(tmpbuf, "@#CLASS_@#FUNC", classname, evname, EINA_TRUE);
        eina_strbuf_replace_all(tmpbuf, ",", "_");
        eina_strbuf_append_printf(str_ev, "\n    %s,", eina_strbuf_string_get(tmpbuf));
     }
   eina_strbuf_replace_all(str_end, "@#list_evdesc", eina_strbuf_string_get(str_ev));

   const char *inherit_name;
   eina_strbuf_reset(tmpbuf);
   EINA_LIST_FOREACH(eolian_class_inherits_list_get(classname), itr, inherit_name)
     {
        if (!strcmp(inherit_name, "Elm_Widget"))
          eina_strbuf_append(tmpbuf, "ELM_OBJ_WIDGET_CLASS, ");
        else if (!strcmp(inherit_name, "Elm_Interface_Scrollable"))
          eina_strbuf_append(tmpbuf, "ELM_SCROLLABLE_INTERFACE, ");
        else
          _template_fill(tmpbuf, "@#OBJCLASS_CLASS, ", inherit_name, "", EINA_FALSE);
     }

   if (eina_strbuf_length_get(tmpbuf) == 0) eina_strbuf_append(tmpbuf,"EO_BASE_CLASS, ");
   eina_strbuf_replace_all(str_end, "@#list_inherit", eina_strbuf_string_get(tmpbuf));

   eina_strbuf_replace_all(str_end, "@#list_func", eina_strbuf_string_get(str_func));
   eina_strbuf_replace_all(str_end, "@#list_op", eina_strbuf_string_get(str_op));
   eina_strbuf_replace_all(str_end, "@#list_ctors_body", eina_strbuf_string_get(str_bodyf));

   eina_strbuf_append(buf, eina_strbuf_string_get(str_end));

   eina_strbuf_free(tmpbuf);
   eina_strbuf_free(str_op);
   eina_strbuf_free(str_func);
   eina_strbuf_free(str_bodyf);
   eina_strbuf_free(str_end);
   eina_strbuf_free(str_ev);

   return EINA_TRUE;
}

