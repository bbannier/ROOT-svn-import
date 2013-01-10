/**
 * afResMon.h -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * Monitor system resources incrementally and differentially.
 */

#ifndef AFRESMON_H
#define AFRESMON_H

#include <fstream>

#include <unistd.h>
//#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <stdio.h>

#include "afLog.h"

#define AFRESMON_BUFSIZE 100

namespace af {

  /** Structure to hold timings.
   */
  typedef struct {
    double real_sec;
    double user_sec;
    double sys_sec;
  } res_timing_t;

  /** Structure to hold memory. 1 KiB = 1024 bytes
   */
  typedef struct {
    unsigned long virt_kib;
    unsigned long rss_kib;
  } res_mem_t;

  /** The actual class.
   */
  class resMon {

    public:
      resMon();
      res_timing_t &get_delta_timing();
      res_timing_t &get_cumul_timing();
      res_mem_t    &get_mem_usage();

    private:

      double get_wall_sec();
      bool fetch_cpu_timing(res_timing_t &rt);
      res_timing_t &get_delta_timing_ref(res_timing_t &ref);

      //struct timespec buf_ts;
      struct timeval buf_tv;
      struct rusage buf_ru;
      char buf[AFRESMON_BUFSIZE];
      std::string procfn;
      pid_t pid;
      res_timing_t time_ref0;
      res_timing_t time_ref1;
      res_timing_t buf_time;
      res_mem_t    buf_mem;

  };

};

#endif // AFRESMON_H
