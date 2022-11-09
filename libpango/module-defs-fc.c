/* Hand-written. Once the GNU configure mechanism is used
 * on Win32, too, can be generated.
*/

#include "modules.h"

#ifdef VAXC
void _pan_b_ft2_extern()
{
}
#endif

#ifdef PANGO_MODULE_PREFIX
/* by defining PANGO_MODULE_PREFIX the basic module gets include in the
 * backend library, here ../modules/basic/basic-fc.c
 * It helps the backend to not fall on its nose even with a screwed module
 * configuration. There should be at least enough fonts available to show
 * an error message ...
 */
void         _pan_b_ft2_script_engine_list   (PangoEngineInfo **engines,
                                                    gint             *n_engines);
void         _pan_b_ft2_script_engine_init   (GTypeModule      *module);
void         _pan_b_ft2_script_engine_exit   (void);
PangoEngine *_pan_b_ft2_script_engine_create (const char       *id);
#endif

PangoIncludedModule _pango_included_fc_modules[] = {
#ifdef PANGO_MODULE_PREFIX
 { 
   _pan_b_ft2_script_engine_list,  
   _pan_b_ft2_script_engine_init, 
   _pan_b_ft2_script_engine_exit,  
   _pan_b_ft2_script_engine_create 
 },
#endif
 { NULL, NULL, NULL },
};
