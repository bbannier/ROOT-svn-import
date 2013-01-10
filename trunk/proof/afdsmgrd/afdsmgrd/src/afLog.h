/**
 * afLog.h -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * Log facility with different error types and error levels. Log file rotation
 * and compression is supported. Every string function used therein is memory
 * safe.
 */

#ifndef AFLOG_H
#define AFLOG_H

#define AF_LOG_BUFSIZE 2000

#include <iostream>
#include <fstream>

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <libgen.h>

namespace af {

  /** Log message level: higher numbers mean higher importance.
   */
  typedef enum {
    log_level_urgent = 4,
    log_level_high   = 3,
    log_level_normal = 2,
    log_level_low    = 1,
    log_level_debug  = 0
  } log_level_t;

  /** Log message type.
   */
  typedef enum {
    log_type_ok,
    log_type_info,
    log_type_warning,
    log_type_error,
    log_type_fatal
  } log_type_t;

  /** Rotate error.
   */
  typedef enum {
    rotate_err_ok = 0,
    rotate_err_rename,
    rotate_err_compress
  } rotate_err_t;

  /** Logging facility. Multiple instances are allowed. The forward declaration
   *  is needed because of the static pointer to an instance of the class
   *  itself.
   */
  class log;
  class log {

    public:

      log(std::ostream &out_stream, log_level_t min_level,
        std::string &banner_msg);
      log(const char *log_file, log_level_t min_level, std::string &banner_msg);
      void set_level(log_level_t min_level) { min_log_level = min_level; };
      virtual ~log();
      void say(log_type_t type, log_level_t level, const char *fmt, ...);
      static void ok(log_level_t level, const char *fmt, ...);
      static void info(log_level_t level, const char *fmt, ...);
      static void warning(log_level_t level, const char *fmt, ...);
      static void error(log_level_t level, const char *fmt, ...);
      static void fatal(log_level_t level, const char *fmt, ...);

    private:

      std::ostream *out;
      std::ofstream *out_file;
      std::string file_name;
      time_t rotated_time;
      double secs_rotate;
      char strbuf[AF_LOG_BUFSIZE];
      static log *stdlog;
      log_level_t min_log_level;
      std::string banner;

      void say_banner(log_type_t type = log_type_info,
        log_level_t level = log_level_high, ...);
      void vsay(log_type_t type, log_level_t level, const char *fmt,
        va_list vargs);
      void rotate_say(log_type_t type, log_level_t level,
        const char *fmt, va_list vargs);
      rotate_err_t rotate();

  };

};

#endif // AFLOG_H
