/**
 * afNotifyApMon.h -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * Plugin that allows for MonALISA notification of datasets and used resources.
 */

#ifndef AFNOTIFYAPMON_H
#define AFNOTIFYAPMON_H

#include <stdio.h>

#include "ApMon.h"

#include "afNotify.h"

#define AF_NOTIFYAPMON_MASH(H, C) ( H = ( (C) ^ \
    ((H << 5) | \
    (H >> (8*sizeof(unsigned int) - 5)))) )

#define AF_NOTIFYAPMON_ID_BUFSIZE 10
#define AF_NOTIFYAPMON_PREFIX_BUFSIZE 110

namespace af {

  class notifyApMon : public notify {

    typedef struct {
      char *host[1];
      char *pwd[1];
      int   port[1];
    } apmon_params_t;

    public:
      notifyApMon(config &_cfg);
      virtual const char *whoami() const;
      virtual void dataset(const char *ds_name, int n_files, int n_staged,
        int n_corrupted, const char *tree_name, int n_events,
        unsigned long long total_size_bytes);
      virtual void resources(unsigned long rss_kib, unsigned long virt_kib,
        float real_sec, float user_sec, float sys_sec,
        float real_delta_sec, float user_delta_sec, float sys_delta_sec);
      virtual void queue(unsigned int n_queued, unsigned int n_runn,
        unsigned int n_success, unsigned int n_fail, unsigned int n_total);
      virtual void commit();
      virtual ~notifyApMon();

    private:
      virtual unsigned int hash(const char *str);
      virtual void apmon_send(char *cluster, char *node, int n_params,
        char **param_names, int *val_types, char **param_vals);
      static void config_apmonurl_callback(const char *dir_name,
        const char *dir_val, void *args);

      ApMon          *apmon;
      void           *apmon_pool;
      apmon_params_t  apmon_params;
      void           *cbk_args[3];
      std::string     apmon_prefix;
      char            id_str_buf[AF_NOTIFYAPMON_ID_BUFSIZE];
      char            prefix_buf[AF_NOTIFYAPMON_PREFIX_BUFSIZE];

      static char          *stat_param_names[];
      static int            stat_val_types[];
      static unsigned int   stat_n_params;
      char                **stat_param_vals;

      struct {
        unsigned int rss_kib;
        unsigned int virt_kib;
        float        uptime_sec;
        float        user_sec;
        float        sys_sec;
        float        pcpu_delta;
        unsigned int n_queued;
        unsigned int n_runn;
        unsigned int n_success;
        unsigned int n_fail;
        unsigned int n_total;
      } stat_vals_pool;

      static char          *ds_param_names[];
      static int            ds_val_types[];
      static unsigned int   ds_n_params;
      char                **ds_param_vals;

      static const char *ds_suffix;
      static const char *stat_suffix;

  };

}

#endif // AFNOTIFYAPMON_H
