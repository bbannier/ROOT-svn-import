/**
 * afExtCmd.h -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * An instance of this class represents and manages an external program that
 * independently runs in background and returns its status on stdout on a single
 * line with separated fields.
 *
 * The class is capable of checking if the program is still running and parses
 * the output, made of fields and values, in memory.
 */

#ifndef AFEXTCMD_H
#define AFEXTCMD_H

#define AF_EXTCMD_BUFSIZE 1000
#define AF_EXTCMD_USLEEP 20000

#include "afLog.h"

#include <map>
#include <string>
#include <fstream>
#include <stdexcept>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>

namespace af {

  typedef std::map<std::string,std::string> fields_t;
  typedef std::pair<std::string,std::string> key_val_t;
  typedef fields_t::const_iterator fields_iter_t;

  class extCmd {

    public:

      extCmd(const char *command, unsigned int id = 0);
      virtual ~extCmd();
      int run();
      bool is_running();
      pid_t get_pid() { return pid; };
      void get_output();
      void print_fields(bool log = false);
      bool is_ok() { return ok; };
      unsigned int get_id() { return id; };
      bool stop();

      unsigned long get_field_uint(const char *key);
      long get_field_int(const char *key);
      double get_field_real(const char *key);
      const char *get_field_text(const char *key);

      inline void set_timeout_secs(unsigned long ts) { timeout_secs = ts; };
      inline void set_stop_grace_secs(unsigned long gs) {
        stop_grace_secs = gs;
        stop_grace_loops = 1000000 * gs / AF_EXTCMD_USLEEP;
      };
      inline long get_timeout_secs() { return timeout_secs; };
      inline long get_stop_grace_secs() { return stop_grace_secs; };

      static void set_helper_path(const char *path);
      static void set_temp_path(const char *path);
      static const char *get_helper_path() { return helper_path.c_str(); };
      static const char *get_temp_path() { return temp_path.c_str(); };

    private:

      bool cleanup();

      char strbuf[AF_EXTCMD_BUFSIZE];
      pid_t pid;
      unsigned int id;
      std::string cmd;
      fields_t fields_map;
      bool ok;
      bool already_started;

      struct timeval start_tv;
      struct timeval now_tv;
      struct stat    buf_stat;

      unsigned long timeout_secs;
      unsigned long stop_grace_secs;
      unsigned long stop_grace_loops;

      static std::string helper_path;
      static std::string temp_path;
      static const char *errf_pref;
      static const char *outf_pref;
      static const char *pidf_pref;

      static bool make_temp_path();

  };

};

#endif // AFEXTCMD_H
