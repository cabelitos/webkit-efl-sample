#include <Ecore.h>
#include <Ecore_Getopt.h>
#include <Ecore_Evas.h>
#include <EWebKit2.h>

#ifndef DEFAULT_URL
#define DEFAULT_URL "http://profusion.mobi/"
#endif

#ifndef DEFAULT_THEME
#define DEFAULT_THEME EWEBKIT2_DATADIR "/themes/default.edj"
#endif

static void
on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
on_js_alert(Ewk_View_Smart_Data *sd, const char *message)
{
   printf("JS ALERT:\n%s\n\n", message);
}

static void
on_web_view_close(Ewk_View_Smart_Data *sd)
{
   ecore_main_loop_quit();
}

static const Ecore_Getopt options = {
  "webkit-efl",
  "%prog [options] [url]",
  "1",
  "(C) 2016 ProFUSION Sistemas e Solucoes LTDA\n",
  "Apache-2",
  "Example WebKit-EFL loaded",
  EINA_TRUE,
  {
    ECORE_GETOPT_STORE_STR('e', "engine", "The Ecore-Evas engine (see --list-engines)"),
    ECORE_GETOPT_CALLBACK_NOARGS('E', "list-engines", "List available Ecore-Evas engines",
                                 ecore_getopt_callback_ecore_evas_list_engines, NULL),
    ECORE_GETOPT_STORE_STR(0, "engine-options", "Extra options to pass to engine, like display=XXX;rotation=90"),

    ECORE_GETOPT_STORE_STR('t', "theme", "Path to Edje (*.edj) file with WebKit-EFL theme. Default=" DEFAULT_THEME),

    ECORE_GETOPT_CALLBACK_ARGS('g', "geometry", "Specify window geometry",
                               "X:Y:WIDTH:HEIGHT",
                               ecore_getopt_callback_geometry_parse,
                               NULL),
    ECORE_GETOPT_STORE_TRUE('f', "fullscreen", "Start in fullscreen."),

    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_SENTINEL
  }
};

int
main(int argc, char *argv[])
{
   const char *url = DEFAULT_URL;
   char *engine = NULL;
   char *engine_options = NULL;
   char *theme = DEFAULT_THEME;
   int args = 1;
   Eina_Rectangle geometry = { 0, 0, 800, 600 };
   Eina_Bool fullscreen = EINA_FALSE;
   Eina_Bool quit_option = EINA_FALSE;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_STR(engine),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_STR(engine_options),

     ECORE_GETOPT_VALUE_STR(theme),

     ECORE_GETOPT_VALUE_PTR_CAST(geometry),
     ECORE_GETOPT_VALUE_BOOL(fullscreen),

     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_NONE
   };
   Ecore_Evas *ee;
   Evas *evas;
   Ewk_View_Smart_Class sc = EWK_VIEW_SMART_CLASS_INIT_NAME_VERSION("Webkit_Efl_Sample_View");
   Evas_Smart *smart;
   Evas_Object *web_view;
   int ret = EXIT_SUCCESS;

   ecore_init();
   ecore_evas_init();
   ewk_init();

   ecore_app_args_set(argc, (const char **)argv);
   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0)
     {
        fputs("ERROR: could not parse options.\n", stderr);
        ret = EXIT_FAILURE;
        goto end;
     }

   if (quit_option)
     {
        ret = EXIT_SUCCESS;
        goto end;
     }

   if (args < argc) url = argv[args];

   ee = ecore_evas_new(engine, geometry.x, geometry.y, geometry.w, geometry.h, engine_options);
   if (!ee)
     {
        fprintf(stderr, "ERROR: could not create window. --engine=%s --engine-options=%s\n", engine ? engine : "", engine_options ? engine_options : "");
        goto end;
     }

   evas = ecore_evas_get(ee);

   ewk_view_smart_class_set(&sc);
   sc.run_javascript_alert = on_js_alert;
   sc.window_close = on_web_view_close;

   smart = evas_smart_class_new(&sc.sc);
   web_view = ewk_view_smart_add(evas, smart,
                                 ewk_context_default_get(),
                                 ewk_page_group_create("main"));
   evas_object_resize(web_view, geometry.w, geometry.h);
   evas_object_show(web_view);
   ecore_evas_object_associate(ee, web_view, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);

   ewk_view_theme_set(web_view, theme);
   ewk_view_url_set(web_view, url);

   ecore_evas_fullscreen_set(ee, fullscreen);
   ecore_evas_show(ee);
   ecore_evas_callback_delete_request_set(ee, on_delete);

   ecore_evas_focus_set(ee, EINA_TRUE);
   evas_object_focus_set(web_view, EINA_TRUE);

   ecore_main_loop_begin();

   ecore_evas_free(ee);

 end:
   ewk_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();
   return ret;
}
