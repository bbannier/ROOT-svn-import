/**
 * afNotify.h -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * This class acts as a wrapper around an external generic notification library
 * with expected function names in a flattened namespace.
 */

#ifndef AFNOTIFY_H
#define AFNOTIFY_H

#include <dlfcn.h>

#include <exception>

#include "afConfig.h"
#include "afLog.h"

namespace af {

  /** The main class of this file.
   */
  class notify {

    typedef notify *(*create_t)(config &);
    typedef void (*destroy_t)(notify *);

    public:

      /** Functions that *must* be implemented by subclasses (pure virtual).
       */
      virtual void dataset(const char *ds_name, int n_files, int n_staged,
        int n_corrupted, const char *tree_name, int n_events,
        unsigned long long total_size_bytes) = 0;
      virtual void resources(unsigned long rss_kib, unsigned long virt_kib,
        float real_sec, float user_sec, float sys_sec,
        float real_delta_sec, float user_delta_sec, float sys_delta_sec) = 0;
      virtual void queue(unsigned int n_queued, unsigned int n_runn,
        unsigned int n_success, unsigned int n_fail, unsigned int n_total) = 0;
      virtual void commit() = 0;
      virtual const char *whoami() const = 0;

      /** Plugin creation and destruction.
       */
      notify(config &_cfg) : cfg(_cfg) {};
      static notify *load(const char *libpath, config &cfg);
      static void unload(notify *notif);
      virtual ~notify() {}

    protected:
      config   &cfg;

    private:
      void     *lib_handler;
      create_t  lib_create;
      destroy_t lib_destroy;

  };

}

#endif // AFNOTIFY_H
