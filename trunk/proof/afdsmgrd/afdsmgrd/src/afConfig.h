/**
 * afConfig.h -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * This class is a collection of key/value pairs associated to a configuration
 * file. If the configuration file changes the values are updated dynamically.
 */

#ifndef AFCONFIG_H
#define AFCONFIG_H

#define AF_CONFIG_BUFSIZE 2000

#include "afRegex.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

/** Constants for directive limits.
 */
#define AF_INT_MAX  ( std::numeric_limits<long>::max() )
#define AF_INT_MIN  ( std::numeric_limits<long>::min() )
#define AF_UINT_MAX ( std::numeric_limits<unsigned long>::max() )
#define AF_UINT_MIN ( 0 )
#define AF_REAL_MAX ( std::numeric_limits<double>::max() )
#define AF_REAL_MIN ( -AF_REAL_MAX )

namespace af {

  /** Identifies a data type of a directive.
   */
  typedef enum {
    dir_type_bool,
    dir_type_int,
    dir_type_uint,
    dir_type_real,
    dir_type_text,
    dir_type_custom
  } dir_type_t;

  /** Holds different data types upon need.
   */
  typedef union {
    bool b;
    long i;
    unsigned long u;
    double r;
    std::string *s;
  } mixed_t;

  /** A single directive, with the corresponding binding, default value, maximum
   *  and minimum allowed value, and directive name.
   */
  class cfg_binding {

    public:

      // Constructors
      cfg_binding(const char *name, bool *_dest_ptr, bool _def_val);
      cfg_binding(const char *name, long *_dest_ptr, long _def_val, long _min,
        long _max);
      cfg_binding(const char *name, unsigned long *_dest_ptr,
        unsigned long _def_val, unsigned long _min, unsigned long _max);
      cfg_binding(const char *name, double *_dest_ptr, double _def_val,
        double _min, double _max);
      cfg_binding(const char *name, std::string *_dest_ptr,
        const char *_def_val);
      cfg_binding(const char *name,
        void (*callback)(const char *name, const char *val, void *args),
        void *args );

      virtual ~cfg_binding();
      inline void *get_dest() const { return dest; };
      inline dir_type_t get_type() const { return type; };
      inline const char *get_name() const { return dir_name.c_str(); };

      void assign_default();
      void assign(const char *value);

      void print() const;
      inline void touch(bool t = true) { touched = t; };
      inline bool is_touched() { return touched; };

    private:

      /** Templatized private inline functions that check for limits and set
       *  the default limits.
       */
      template<typename T>
        static inline bool fits_limits(T val, T inf, T sup) {
          return ((val >= inf) && (val <= sup));
        };
      template<typename T>
        void inline ctor_helper(T _def_val, T _min, T _max);

      std::string dir_name;
      union {
        void *dest;
        void *callback_args;
      };
      bool touched;
      dir_type_t type;
      union {
        mixed_t min;
        std::string *prev_val;
      };
      union {
        mixed_t max;
        bool first_callback;
      };
      mixed_t def_val;
      void (*ext_callback)(const char *name, const char *val, void *args);

      static const char *true_str[];
      static const char *false_str[];
      static size_t true_str_len;
      static size_t false_str_len;

  };

  /** Useful typedefs.
   */
  typedef std::vector<cfg_binding *> conf_dirs_t;
  typedef conf_dirs_t::iterator conf_dirs_iter_t;

  /** Manages a configuration file.
   */
  class config {

    public:

      config(const char *config_file);
      virtual ~config();
      void print_bindings();
      void read_file();
      void default_all();
      bool update();
      bool unbind(const char *dir_name);
      void unbind_all();

      /** Bindings.
       */
      // TODO: Think about a way to avoid automatic casts (e.g. int to long)!
      void bind_bool(const char *dir_name, bool *dest_ptr, bool def_val);
      void bind_int(const char *dir_name, long *dest_ptr, long def_val,
        long min_eq, long max_eq);
      void bind_uint(const char *dir_name, unsigned long *dest_ptr,
        unsigned long def_val, unsigned long min_eq, unsigned long max_eq);
      void bind_real(const char *dir_name, double *dest_ptr, double def_val,
        double min_eq, double max_eq);
      void bind_text(const char *dir_name, std::string *dest_ptr,
        const char *def_val);
      void bind_callback(const char *dir_name,
        void (*callback)(const char *name, const char *val, void *args),
        void *args);

    private:

      inline char *ltrim(char *str) const;
      inline char *rtrim(char *str) const;

      conf_dirs_t directives;
      std::string file_name;
      char strbuf[AF_CONFIG_BUFSIZE];
      struct stat file_stat;
      bool never_read_before;

  };

};

#endif // AFCONFIG_H
