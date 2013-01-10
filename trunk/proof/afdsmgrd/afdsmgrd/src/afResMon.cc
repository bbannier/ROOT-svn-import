/**
 * afResMon.cc -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * See header file for a description of the class.
 */

#include "afResMon.h"

using namespace af;

/** Constructor: initializes current PID and reference timings.
 */
resMon::resMon() {

  // PID-related initializations
  pid = getpid();
  snprintf(buf, AFRESMON_BUFSIZE, "/proc/%d/statm", pid);
  procfn = buf;

  // Initializes timing reference for "cumulative" requests
  time_ref0.real_sec = get_wall_sec();
  fetch_cpu_timing(time_ref0);

  // Initializes timing reference for "delta" requests
  memcpy(&time_ref1, &time_ref0, sizeof(res_timing_t));
}

/** Returns CPU timing with respect to the last call of this very function
 *  (delta values).
 */
res_timing_t &resMon::get_delta_timing() {
  res_timing_t &rt = get_delta_timing_ref(time_ref1);  // in buf_time
  fetch_cpu_timing(time_ref1);
  time_ref1.real_sec = get_wall_sec();
  return rt;
}

/** Returns CPU timing with respect to the initialization of this class
 *  (cumulative values).
 */
res_timing_t &resMon::get_cumul_timing() {
  return get_delta_timing_ref(time_ref0);
}

/** Reads memory information (resident and virtual) from the /proc filesystem.
 */
res_mem_t &resMon::get_mem_usage() {
  std::ifstream fp(procfn.c_str());
  if (fp) {
    fp.getline(buf, 100);
    sscanf(buf, "%lu %lu", &(buf_mem.virt_kib), &(buf_mem.rss_kib));
    buf_mem.virt_kib *= getpagesize() / 1024;
    buf_mem.rss_kib *= getpagesize() / 1024;
    fp.close();
  }
  else {
    buf_mem.virt_kib = 0;
    buf_mem.rss_kib = 0;
  }
  return buf_mem;
}

/** Gets real time, CPU user time and CPU system time (all in seconds) with
 *  respect to a certain reference timing.
 */
res_timing_t &resMon::get_delta_timing_ref(res_timing_t &ref) {
  buf_time.real_sec = get_wall_sec() - ref.real_sec;
  if (!fetch_cpu_timing(buf_time)) {
    buf_time.user_sec = -1.;
    buf_time.sys_sec = -1.;
  }
  else {
    buf_time.user_sec -= ref.user_sec;
    buf_time.sys_sec -= ref.sys_sec;
  }
  return buf_time;
}

/** Gets CPU timings and transforms them in seconds. Results are saved in
 *  the specified struct. Returns true on success, false on failure. On failure,
 *  user and system CPU time are set to zero in the destination struct.
 */
bool resMon::fetch_cpu_timing(res_timing_t &rt) {
  if (getrusage(RUSAGE_SELF, &buf_ru) < 0) {
    rt.user_sec = 0;
    rt.sys_sec = 0;
    return false;
  }
  rt.user_sec = (double)buf_ru.ru_utime.tv_sec + \
    (double)buf_ru.ru_utime.tv_usec / 1000000.;
  rt.sys_sec = (double)buf_ru.ru_stime.tv_sec + \
    (double)buf_ru.ru_stime.tv_usec / 1000000.;
  return true;
}

/** Gets value of an arbitrarily-started timer, in seconds. The chosen timer's
 *  absolute value is meaningless: relative values (i.e., differences) are to be
 *  considered. The selected timer is chosen to be resilient to system time
 *  modifications.
 */
double resMon::get_wall_sec() {
  //clock_gettime(CLOCK_MONOTONIC, &buf_ts);
  //return (double)buf_ts.tv_sec + (double)buf_ts.tv_nsec / 1000000000.;
  gettimeofday(&buf_tv, 0);
  return (double)buf_tv.tv_sec + (double)buf_tv.tv_usec / 1000000.;
}
