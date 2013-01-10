/**
 * afNotifyApMon.cc -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * See header file for a description of the class.
 */

#include "afNotifyApMon.h"

using namespace af;

/** Static variables: suffixes for "cluster" name for datasets and daemon
 *  status. They are appended to the value specified in the apmonprefix
 *  directive on file.
 */
const char *notifyApMon::ds_suffix = "_datasets";
const char *notifyApMon::stat_suffix = "_status";

/** Arrays used by ApMon::sendParameters().
 */

char *notifyApMon::stat_param_names[] = {
  (char *)"rss_kib",
  (char *)"virt_kib",
  (char *)"uptime_sec",
  (char *)"user_sec",
  (char *)"sys_sec",
  (char *)"pcpu_delta",
  (char *)"queue_queued",
  (char *)"queue_running",
  (char *)"queue_success",
  (char *)"queue_failed",
  (char *)"queue_total"
};

int notifyApMon::stat_val_types[] = {
  XDR_INT32,
  XDR_INT32,
  XDR_REAL32,
  XDR_REAL32,
  XDR_REAL32,
  XDR_REAL32,
  XDR_INT32,
  XDR_INT32,
  XDR_INT32,
  XDR_INT32,
  XDR_INT32
};

unsigned int notifyApMon::stat_n_params = sizeof(stat_val_types)/sizeof(int);

char *notifyApMon::ds_param_names[] = {
  (char *)"dsname",
  (char *)"filescount",
  (char *)"stagecount",
  (char *)"corruptedcount",
  (char *)"stagedpct",
  (char *)"corruptedpct",
  (char *)"treename",
  (char *)"nevts",
  (char *)"totsizemb" // mb is for MiB, i.e. 1024*1024 bytes
};

int notifyApMon::ds_val_types[] = {
  XDR_STRING,
  XDR_INT32,
  XDR_INT32,
  XDR_INT32,
  XDR_REAL32,
  XDR_REAL32,
  XDR_STRING,
  XDR_INT32,
  XDR_INT32
};

unsigned int notifyApMon::ds_n_params = sizeof(ds_val_types)/sizeof(int);

/** Non-member function with C-style name binding (i.e., no mangling) to
 *  allow for classes in libraries through polymorphism.
 */
extern "C" notify *create(config &_cfg) {
  return new notifyApMon(_cfg);
}

/** Non-member destructor with C-style name binding. It is to be used instead of
 *  delete from the caller: the caller _has_ indeed the knowledge to walk
 *  through the destruction functions (because of virtual dtors), but when it's
 *  time to actually _free_ the memory (after dtors have been called), the
 *  caller has no effective knowledge on the size of the class! So, it is
 *  totally not sufficient to declare a virtual destructor: the destructor must
 *  be invoked with "delete" but from a function *within* this library!
 */
extern "C" void destroy(notify *notif) {
  delete notif;
}

/** Returns a string identifier for this plugin.
 */
const char *notifyApMon::whoami() const {
  return "ApMon (MonALISA) notification plugin";
}

/** Default constructor.
 */
notifyApMon::notifyApMon(config &_cfg) : notify(_cfg) {

  apmon = NULL;
  apmon_pool = malloc( sizeof(ApMon) );  // pool-allocating memory for ApMon

  apmon_params.port[0] = -1;
  apmon_params.host[0] = NULL; //(char *)0x2;
  apmon_params.pwd[0]  = NULL; //(char *)0x3;

  cbk_args[0] = &apmon;         // ApMon **
  cbk_args[1] = apmon_pool;     // void *
  cbk_args[2] = &apmon_params;  // apmon_params_t *

  // Bind directives to the configuration file
  cfg.bind_callback("dsmgrd.apmonurl", notifyApMon::config_apmonurl_callback,
    cbk_args);
  cfg.bind_text("dsmgrd.apmonprefix", &apmon_prefix, "");

  // Allocate array for parameters and init it to zero: for info, see:
  // http://stackoverflow.com/questions/2204176/
  // how-to-initialise-memory-with-new-operator-in-c
  stat_param_vals = new char *[stat_n_params];
  ds_param_vals = new char *[ds_n_params]();

  // Pointers to values inside a pool
  stat_param_vals[0]  = (char *)&(stat_vals_pool.rss_kib);
  stat_param_vals[1]  = (char *)&(stat_vals_pool.virt_kib);
  stat_param_vals[2]  = (char *)&(stat_vals_pool.uptime_sec);
  stat_param_vals[3]  = (char *)&(stat_vals_pool.user_sec);
  stat_param_vals[4]  = (char *)&(stat_vals_pool.sys_sec);
  stat_param_vals[5]  = (char *)&(stat_vals_pool.pcpu_delta);
  stat_param_vals[6]  = (char *)&(stat_vals_pool.n_queued);
  stat_param_vals[7]  = (char *)&(stat_vals_pool.n_runn);
  stat_param_vals[8]  = (char *)&(stat_vals_pool.n_success);
  stat_param_vals[9]  = (char *)&(stat_vals_pool.n_fail);
  stat_param_vals[10] = (char *)&(stat_vals_pool.n_total);

}

/** Destructor: frees memory (if used), then frees the pool.
 */
notifyApMon::~notifyApMon() {

  if (apmon) apmon->~ApMon();
  if (apmon_params.host[0]) free(apmon_params.host[0]);
  if (apmon_params.pwd[0]) free(apmon_params.pwd[0]);

  free(apmon_pool);

  cfg.unbind("dsmgrd.apmonurl");
  cfg.unbind("dsmgrd.apmonprefix");

  delete[] stat_param_vals;
  delete[] ds_param_vals;

}

/** Notifies to MonALISA the presence of a dataset.
 */
void notifyApMon::dataset(const char *ds_name, int n_files, int n_staged,
  int n_corrupted, const char *tree_name, int n_events,
  unsigned long long total_size_bytes) {

  if ((!apmon) || (apmon_prefix.empty())) {
    log::warning(log_level_high,
      "Skipping MonALISA datasets notification due to invalid configuration");
    return;
  }

  float pct_stg = 100. * n_staged / n_files;
  float pct_cor = 100. * n_corrupted / n_files;

  int total_size_mib = (int)(total_size_bytes / 1048576L);

  ds_param_vals[0] = (char *)ds_name;
  ds_param_vals[1] = (char *)&n_files;
  ds_param_vals[2] = (char *)&n_staged;
  ds_param_vals[3] = (char *)&n_corrupted;
  ds_param_vals[4] = (char *)&pct_stg;
  ds_param_vals[5] = (char *)&pct_cor;
  ds_param_vals[6] = (char *)tree_name;
  ds_param_vals[7] = (char *)&n_events;
  ds_param_vals[8] = (char *)&total_size_mib;

  snprintf(id_str_buf, AF_NOTIFYAPMON_ID_BUFSIZE, "%08x", hash(ds_name));
  snprintf(prefix_buf, AF_NOTIFYAPMON_PREFIX_BUFSIZE, "%s%s",
    apmon_prefix.c_str(), ds_suffix);

  apmon_send(prefix_buf, (char *)id_str_buf,
    ds_n_params, ds_param_names, ds_val_types, ds_param_vals);

}

/** Report resources usage. Note: a call to commit() is required to send info to
 *  ApMon.
 */
void notifyApMon::resources(unsigned long rss_kib, unsigned long virt_kib,
  float real_sec, float user_sec, float sys_sec,
  float real_delta_sec, float user_delta_sec, float sys_delta_sec) {
  stat_vals_pool.rss_kib    = rss_kib;
  stat_vals_pool.virt_kib   = virt_kib;
  stat_vals_pool.uptime_sec = real_sec;
  stat_vals_pool.user_sec   = user_sec;
  stat_vals_pool.sys_sec    = sys_sec;
  stat_vals_pool.pcpu_delta = 100. * user_delta_sec / real_delta_sec;
}

/** Report queue status. Note: a call to commit() is required to send to ApMon.
 */
void notifyApMon::queue(unsigned int n_queued, unsigned int n_runn,
  unsigned int n_success, unsigned int n_fail, unsigned int n_total) {
  stat_vals_pool.n_queued  = n_queued;
  stat_vals_pool.n_runn    = n_runn;
  stat_vals_pool.n_success = n_success;
  stat_vals_pool.n_fail    = n_fail;
  stat_vals_pool.n_total   = n_total;
}

/** Commits to MonALISA data collected through queue() and resources(). Datasets
 *  data needn't this because it is sent immediately.
 */
void notifyApMon::commit() {

  if ((!apmon) || (apmon_prefix.empty())) {
    log::warning(log_level_high,
      "Skipping MonALISA status notification due to invalid configuration");
    return;
  }

  // We suppose that valid data is inside stat_param_vals! Beware!

  snprintf(prefix_buf, AF_NOTIFYAPMON_PREFIX_BUFSIZE, "%s%s",
    apmon_prefix.c_str(), stat_suffix);

  apmon_send(prefix_buf, (char *)"afdsmgrd",
    stat_n_params, stat_param_names, stat_val_types, stat_param_vals);

  // Eventually reset cache
  memset(&stat_vals_pool, 0, sizeof(stat_vals_pool));

}

/** Wrapper around ApMon's sendParameters() that absorbs any error or exception.
 */
void notifyApMon::apmon_send(char *cluster, char *node, int n_params,
  char **param_names, int *val_types, char **param_vals) {

  try {

    // TODO: can config it... or find a better solution (i.e. increase num. of
    // datagrams per second)
    const unsigned int max_retries = 5;
    int r;

    for (unsigned int i=0; i<max_retries; i++) {
      r = apmon->sendParameters(cluster, node, n_params, param_names,
        val_types, param_vals);
      if (r != RET_NOT_SENT) break;
    }

    // Report error only if fails to send message after max_retries
    if (r == RET_NOT_SENT) {
      log::error(log_level_low, "MonALISA notification skipped after %u tries: "
        "maximum number of datagrams per second exceeded", max_retries);
    }

  }
  catch (runtime_error &e) {
    log::error(log_level_high, "Error sending information to MonALISA");
  }

}

/** Callback called every time the dsmgrd.apmonurl changes: it deletes the
 *  current instance of ApMon and creates a new one (if parameters are OK) using
 *  new parameters. Memory for the ApMon object is pool-allocated only once in
 *  the constructor. Note: this function is static and has to be like this,
 *  because a pointer to it is given to the af::config class.
 */
void notifyApMon::config_apmonurl_callback(const char *dir_name,
  const char *dir_val, void *args) {

  void **args_array  = (void **)args;
  ApMon **apmon_ptr = (ApMon **)args_array[0];
  void *apmon_mempool = (void *)args_array[1];
  apmon_params_t *apmon_params_ptr = (apmon_params_t *)args_array[2];

  //
  // Delete previous instance of ApMon, if there is one
  //

  if (*apmon_ptr) {
    log::info(log_level_normal,
      "Configuration changed: removing former ApMon notifier");
    (*apmon_ptr)->~ApMon();  // don't free, just destroy: it's a pool
    *apmon_ptr = NULL; 
  }

  if (apmon_params_ptr->host[0]) {
    free( apmon_params_ptr->host[0]);
    apmon_params_ptr->host[0] = NULL;
  }

  if (apmon_params_ptr->pwd[0]) {
    free(apmon_params_ptr->pwd[0]);
    apmon_params_ptr->pwd[0] = NULL;
  }

  apmon_params_ptr->port[0] = -1;

  //
  // URL parsing: if directive is null, exit here leaving with no valid ApMon
  //

  if (dir_val == NULL) return;

  const char *reurl_ptn =
    "^([a-z]+)://(([^:@]+)?(:([^:@]*))?@)?([^:/]+)(:([0-9]+))?(.*)$";
  string url = dir_val;

  regex_t *re_compd = new regex_t;
  regcomp(re_compd, reurl_ptn, REG_EXTENDED);

  // Parameters for ApMon
  std::string proto, pwd, host;
  int port = -1;

  const unsigned int nmatch = 10; // 9 + 1

  regmatch_t match[nmatch];
  if (regexec(re_compd, url.c_str(), nmatch, match, 0) == 0) {

    // URL is valid

    for (unsigned int i=0; i<nmatch; i++) {
      int s = (int)match[i].rm_so;
      int e = (int)match[i].rm_eo;
      if (s > -1) {
        std::string sub = url.substr(s, e-s);
        //printf("#%d : [%3d, %3d) : {%s}\n", i, s, e, sub.c_str());
        switch (i) {
          case 1: proto = sub; break;
          case 5: pwd   = sub; break;
          case 6: host  = sub; break;
          case 8:
            port = (int)strtol(sub.c_str(), NULL, 0);
          break;
        }
      }
    }

    //
    // Now we have proto, pwd, host and port: create a new ApMon instance
    //

    // Memory is allocated in pool and zeroed because ApMon is deeply bugged!
    memset(apmon_mempool, 0, sizeof(ApMon));

    if (proto == "apmon") {

      // Check port and assign default (8884) if needed
      if ((port <= 0) || (port > 65535)) port = 8884;

      log::info(log_level_normal,
        "Creating new ApMon instance from direct server specification: "
        "host=%s, port=%d, pwd=%s", host.c_str(), port, pwd.c_str());

      apmon_params_ptr->host[0] = strdup(host.c_str());
      apmon_params_ptr->pwd[0]  = strdup(pwd.c_str());
      apmon_params_ptr->port[0] = port;

      // In memory pool
      // ApMon(int nDestinations, char **destAddresses, int *destPorts,
      //   char **destPasswds)
      *apmon_ptr = new(apmon_mempool) ApMon(1,
        apmon_params_ptr->host,
        apmon_params_ptr->port,
        apmon_params_ptr->pwd);

    }
    else {
      log::info(log_level_normal,
        "Creating new ApMon instance from configuration URL: %s", url.c_str());

      apmon_params_ptr->host[0] = strdup(url.c_str());

      // In memory pool
      // ApMon(char *initsource);
      *apmon_ptr = new(apmon_mempool) ApMon(1, apmon_params_ptr->host);
    }

  }

  //
  // If URL is not valid, nothing is done: no valid ApMon will be available
  //

  // Free regex
  regfree(re_compd);
  delete re_compd;

}

/** Hash function identical (i.e., copied and adapted) to the one in ROOT's
 *  TString. Used to hash dataset name.
 */
unsigned int notifyApMon::hash(const char *str) {

  unsigned int len      = (unsigned int)strlen(str);
  unsigned int hv       = len;  // mix in the string length
  unsigned int i        = hv * sizeof(char) / sizeof(unsigned int);
  const unsigned int *p = (const unsigned int *)str;

  while (i--) AF_NOTIFYAPMON_MASH(hv, *p++);  // xor in the chars

  // xor in any remaining char

  if ((i = len*sizeof(char)%sizeof(unsigned int)) != 0) {
    unsigned int h = 0;
    const char* c = (const char*)p;
    while (i--) {
      h = ((h << 8 * sizeof(char)) | *c++);
    }
    AF_NOTIFYAPMON_MASH(hv, h);
  }

  return hv;
}
