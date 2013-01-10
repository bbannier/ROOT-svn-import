/**
 * afLog.cc -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * See header file for a description of the class.
 */

#include "afLog.h"

using namespace af;

/** Static pointer to the standard log facility.
 */
log *log::stdlog = NULL;

/** Constructor. It takes an ostream (NOT ofstream!) as the only argument. Log
 *  rotation is obviously not supported by a generic ostream, only by files. The
 *  given banner, if non-empty, is printed on initialization.
 */
log::log(std::ostream &out_stream, log_level_t min_level,
  std::string &banner_msg) :
  out(&out_stream), out_file(NULL), min_log_level(min_level), rotated_time(0),
  secs_rotate(0.), banner(banner_msg) {
  if (!stdlog) stdlog = this;
  say_banner();
}

/** Constructor. It takes an file name as the only argument. Remember that this
 *  class throws an exception any time a file open call fails (because disk is
 *  full or no permissions on output file or whatever). A banner (if non-empty)
 *  is printed on initialization.
 *
 *  By default, the log file is rotated every 12 hours.
 *
 *  See http://www.cplusplus.com/reference/iostream/ios/exceptions/
 */
log::log(const char *log_file, log_level_t min_level, std::string &banner_msg) :
  file_name(log_file), min_log_level(min_level), rotated_time(time(NULL)),
  secs_rotate(43200.), banner(banner_msg) {

  out_file = new std::ofstream();
  out_file->exceptions(std::ios::failbit);
  out_file->open(log_file, std::ios::app);
  out = out_file;

  if (!stdlog) stdlog = this;

  say_banner();
}

/** Destructor. Sets to NULL the default facility if it equals to the current
 *  one.
 */
log::~log() {
  if (out_file) {
    out_file->close();
    delete out_file;
  }
  if (this == stdlog) stdlog = NULL;
}

/** Prints out the banner, if non-empty. Varargs are for compatibility and are
 *  not used actually. This function does not trigger log rotation, even if
 *  needed.
 */
void log::say_banner(log_type_t type, log_level_t level, ...) {
  if (!banner.empty()) {
    va_list vargs;
    va_start(vargs, level);
    vsay(type, level, banner.c_str(), vargs);
    va_end(vargs);
  }
}

/** Says a log message with the given type and level. This is the main function
 *  of the class.
 */
void log::say(log_type_t type, log_level_t level, const char *fmt, ...) {
  va_list vargs;
  va_start(vargs, fmt);
  rotate_say(type, level, fmt, vargs);
  va_end(vargs);
};

/** Rotates the logfile and returns a value of type rotate_err_t. Keep in mind
 *  that if file open fails an exception is thrown and must be caught, elsewhere
 *  the program aborts. See the constructor (for files, not generic ostreams)
 *  for more information.
 */
rotate_err_t log::rotate() {

  struct tm *rotated_tm = localtime(&rotated_time);

  // Formatted date/time
  static char datetime_fmt[20];
  strftime(datetime_fmt, 19, "%Y%m%d-%H%M%S", rotated_tm);

  // Compose archive log file name (the uncompressed one) on strbuf
  snprintf(strbuf, AF_LOG_BUFSIZE, "%s-%s", file_name.c_str(), datetime_fmt);

  out_file->close();

  rotate_err_t ret = rotate_err_ok;

  if (rename(file_name.c_str(), strbuf)) ret = rotate_err_rename;
  else {

    // Compose compress command
    snprintf(strbuf, AF_LOG_BUFSIZE, "bzip2 -9 \"%s-%s\" > /dev/null 2>&1",
      file_name.c_str(), datetime_fmt);

    if (system(strbuf)) ret = rotate_err_compress;
  }

  out_file->open(file_name.c_str(), std::ios::app);  // might throw an exception

  return ret;
}

/** Private function that checks if the current stream is rotateable and should
 *  be rotated, rotates it in such a case, then says the message to the logfile
 *  with the appropriate level and type.
 */
void log::rotate_say(log_type_t type, log_level_t level, const char *fmt,
  va_list vargs) {

  if (out_file) {

    time_t cur_time = time(NULL);

    if (difftime(cur_time, rotated_time) >= secs_rotate) {

      switch (rotate()) {
        case rotate_err_rename:
          vsay(log_type_error, log_level_urgent,
            "Can't rename logfile: rotation failed", vargs);
        break;

        case rotate_err_compress:
          vsay(log_type_warning, log_level_urgent,
            "Can't compress rotated logfile", vargs);
        break;

        case rotate_err_ok:
          say_banner();
          vsay(log_type_ok, log_level_urgent, "Logfile rotated", vargs);
        break;
      }

      rotated_time = cur_time;

    }
  }

  vsay(type, level, fmt, vargs);
}

/** Says a log message, varargs version. This function is private and used
 *  internally to format the messages.
 */
void log::vsay(log_type_t type, log_level_t level, const char *fmt,
  va_list vargs) {

  if (level < min_log_level) return;

  char pref;
  static char color[8] = "";

  switch (type) {
    case log_type_ok:      pref = 'O'; strcpy(color, "\033[1;32m"); break;
    case log_type_info:    pref = 'I'; strcpy(color, "\033[1;36m"); break;
    case log_type_warning: pref = 'W'; strcpy(color, "\033[1;33m"); break;
    case log_type_error:   pref = 'E'; strcpy(color, "\033[1;31m"); break;
    case log_type_fatal:   pref = 'F'; strcpy(color, "\033[1;35m"); break;
  }

  time_t cur_time = time(NULL);
  struct tm *cur_tm = localtime(&cur_time);
  strftime(strbuf, AF_LOG_BUFSIZE, "-[%Y%m%d-%H%M%S]", cur_tm);
  if (!out_file) *out << color << pref << strbuf << " \033[m";
  else *out << pref << strbuf << " ";

  vsnprintf(strbuf, AF_LOG_BUFSIZE, fmt, vargs);
  *out << strbuf << std::endl;
}

/** Success message of the specified log level on the default log facility.
 */
void log::ok(log_level_t level, const char *fmt, ...) {
  if (!stdlog) return;
  va_list vargs;
  va_start(vargs, fmt);
  stdlog->rotate_say(log_type_ok, level, fmt, vargs);
  va_end(vargs);
};

/** Info message of the specified log level on the default log facility.
 */
void log::info(log_level_t level, const char *fmt, ...) {
  if (!stdlog) return;
  va_list vargs;
  va_start(vargs, fmt);
  stdlog->rotate_say(log_type_info, level, fmt, vargs);
  va_end(vargs);
};

/** Warning message of the specified log level on the default log facility.
 */
void log::warning(log_level_t level, const char *fmt, ...) {
  if (!stdlog) return;
  va_list vargs;
  va_start(vargs, fmt);
  stdlog->rotate_say(log_type_warning, level, fmt, vargs);
  va_end(vargs);
};

/** Error message of the specified log level on the default log facility.
 */
void log::error(log_level_t level, const char *fmt, ...) {
  if (!stdlog) return;
  va_list vargs;
  va_start(vargs, fmt);
  stdlog->rotate_say(log_type_error, level, fmt, vargs);
  va_end(vargs);
};

/** Fatal error message of the specified log level on the default log facility.
 */
void log::fatal(log_level_t level, const char *fmt, ...) {
  if (!stdlog) return;
  va_list vargs;
  va_start(vargs, fmt);
  stdlog->rotate_say(log_type_fatal, level, fmt, vargs);
  va_end(vargs);
};
