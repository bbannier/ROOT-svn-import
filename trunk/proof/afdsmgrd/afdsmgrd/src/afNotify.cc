/**
 * afNotify.cc -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * See header file for a description of the class.
 */

#include "afNotify.h"

using namespace af;

/** Loads the specialized notification library, which is implemented as a
 *  subclass of af::notify. This function is declared as static.
 */
notify *notify::load(const char *libpath, config &cfg) {

  void *lib_handler = dlopen(libpath, RTLD_LAZY);
  if (!lib_handler) {
    af::log::error(af::log_level_high, "dlopen() failed: %s", dlerror());
    return NULL;
  }

  const char *dlerr;

  create_t lib_create = (create_t)dlsym(lib_handler, "create");
  if (dlerr = dlerror()) {
    af::log::error(af::log_level_high, "dlsym() can't find create()!");
    return NULL;
  }

  destroy_t lib_destroy = (destroy_t)dlsym(lib_handler, "destroy");
  if (dlerr = dlerror()) {
    af::log::error(af::log_level_high, "dlsym() can't find destroy()!");
    return NULL;
  }

  af::notify *notif = lib_create(cfg);
  if (notif) {
    notif->lib_handler = lib_handler;
    notif->lib_create  = lib_create;
    notif->lib_destroy = lib_destroy;
  }

  return notif;
}

/** Unloads the specialized notification library given as an instance of a
 *  subclass of notify. This function is declared as static.
 */
void notify::unload(notify *notif) {
  if (!notif) return;
  void *lib_handler = notif->lib_handler;
  notif->lib_destroy(notif);
  dlclose(lib_handler);
}
