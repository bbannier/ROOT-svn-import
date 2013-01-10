/**
 * test.cc -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * Binary used for tests.
 *
 */
#include <stdio.h>
#include <libgen.h>
#include <dlfcn.h>

#include <iostream>
#include <stdexcept>
#include <map>

#include <TDataSetManagerFile.h>
#include <TFileCollection.h>
#include <THashList.h>
#include <TFileInfo.h>

#include "afDataSetList.h"
#include "afOpQueue.h"
#include "afExtCmd.h"
#include "afConfig.h"
#include "afLog.h"
#include "afRegex.h"
#include "afNotify.h"
#include "afOptions.h"

/** Program name that goes in version banner.
 */
#define AF_PROG_NAME "afdsmgrd-test-libs"

/** Waits for user input (debug)
 */
//#define WAIT_USER
void wait_user() {
#ifdef WAIT_USER
  printf("*** waiting for user input ***");
  std::string fuffa;
  std::cin >> fuffa;
#endif
}

bool quit_requested = false;

/** Manages the stop signal (kill -15)
 */
void signal_quit_callback(int signum) {
  printf("quit requested with signal %d\n", signum);
  quit_requested = true;
}

/** Returns a string containing the base path (without trailing slashes) of the
 *  command executed as "cmd" from current working directory. The returned
 *  pointer is dynamically allocated and must be freed by the caller. If some
 *  memory allocation fails, it returns a NULL pointer.
 */
char *exec_path(const char *cmd) {

  char *base = dirname((char *)cmd);  // dirname() returns a ptr to static buf
  char *path;
  size_t maxlen;

  if (base[0] == '/') {
    path = strdup(base);
    if (!path) return NULL;
  }
  else {
    path = getcwd(NULL, 0);  // getcwd() uses malloc()
    if (!path) return NULL;
    if (strcmp(base, ".") != 0) {
      maxlen = strlen(path) + strlen(base) + 2;  // one slash plus one '\0'
      path = (char *)realloc(path, maxlen);
      if (!path) {
        free(path);  // realloc() leaves prev memory unchanged upon failure
        return NULL;
      }
      strncat(path, "/", maxlen);
      strncat(path, base, maxlen);
    }
  }

  return path;
}

/** A callback function to test the bind_callback() feature of af::config.
 */
void directive_callback(const char *name, const char *val, void *args) {
  void **array = (void **)args;

  printf("%s --> ", name);

  if (val == NULL) {
    printf("\033[1;31mcallback for default value called\033[m\n");
  }
  else {
    printf("\033[1;31mdirective callback called:\033[m directive is {%s}, "
      "while the meaning of life the universe and everything remains %ld "
      "and a generic text to remember is \"%s\"\n",
      val, *(long *)array[0], (const char *)array[1]);
  }
}

/** Test dataset manipulation facility.
 */
void test_dsmanip() {

  TDataSetManagerFile *prfdsm = new TDataSetManagerFile(NULL, NULL,
    "dir:/home/volpe/storage/datasets");

  af::dataSetList dsm( prfdsm );

  const char *s;
  TFileInfo *fi;
  TUrl *turl;

  dsm.fetch_datasets();
  while (s = dsm.next_dataset()) {
    printf("--> %s\n", s);

    dsm.fetch_files(NULL);
    while (fi = dsm.next_file()) {
      TFileInfoMeta *meta = fi->GetMetaData(NULL);
      long evts = -1;
      if (meta) evts = meta->GetEntries();
      printf("    entry (%d urls, %ld evts):\n", fi->GetNUrls(), evts);
      fi->ResetUrl();
      while (turl = fi->NextUrl()) printf("     - %s\n", turl->GetUrl());
    }
    dsm.free_files();

  }

  dsm.free_datasets();

  delete prfdsm;

}

/** Test external command facility.
 */
void test_extcmd(const char *argv0) {

  // Find path of current executable (where to find helper executables)
  // This piece of code might be useful, do not remove it, leave it commented.
  {
    char *path = exec_path(argv0);
    const char *helper_exec = "/../libexec/afdsmgrd-exec-wrapper";
    path = (char *)realloc(path, strlen(path)+strlen(helper_exec)+1);
    strcat(path, helper_exec);
    af::extCmd::set_helper_path(path);
    af::extCmd::set_temp_path("/tmp/afdsmgrd");
    free(path);
  }

  //af::extCmd my_cmd("../../src/donothing.sh 1");
  //af::extCmd my_cmd("xrdstagetool -d 2 root://pmaster.to.infn.it//alice/data/2010/LHC10h/000137161/ESDs/pass1_5plus/10000137161031.990/root_archive.zip#AliESDs.root");

  //af::extCmd my_cmd("root.exe -b -q /opt/afdsmgrd/devel/dest/libexec/afdsmgrd-macros/StageXrd.C'(\"root://pmaster.to.infn.it//alice/data/2010/LHC10h/000137161/ESDs/pass1_5plus/10000137161031.990/root_archive.zip#AliESDs.root\", \"/esdTree\")'");
  af::extCmd my_cmd("/tmp/do_nothing.sh");

  my_cmd.run();
  /*
  if (!my_cmd.run()) {
    std::cout << "Cannot start program!" << std::endl;
    return;
  }*/

  std::cout << "Downloading file" << std::flush;

  while (my_cmd.is_running()) {
    std::cout << '.' << std::flush;
    usleep(500000);
  }

  my_cmd.get_output();

  if (my_cmd.is_ok()) {
    unsigned long size_bytes = my_cmd.get_field_uint("Size");
    const char *file_url = my_cmd.get_field_text("");
    printf("done (%ld bytes big)\n", size_bytes);
    printf("Returned file name: %s\n", file_url);

    printf("\n\n\n*** All fields ***\n");
    my_cmd.print_fields();
    printf("\n\n");

  }
  else printf("\nDownload failed\n");

}


/** Test log facility.
 */
void test_log(unsigned long max_iters = 10) {

  std::string banner = AF_VERSION_BANNER;

  af::log log(std::cout, af::log_level_normal, banner);
  unsigned long iter = 0;

  while (true) {
    af::log::info(af::log_level_normal, "Iteration #%lu", iter++);
    sleep(1);
    if ((quit_requested) || (iter == max_iters)) break;
  }

}

/** Test configuration file facility.
 */
void test_config(unsigned int iter_limits = 20) {

  // Configuration file management
  af::config cfg("/opt/afdsmgrd/trunk/dest/etc/afdsmgrd.conf");

  // Directives (bound)
  long test_int = -1;
  std::string test_text;
  double test_real;
  bool test_bool;

  long tmoltuae = 42;
  const char *some_text = "don't panic";
  void *args_to_cbk[] = { &tmoltuae, (void *)some_text };

  cfg.bind_int("signed_integer.directive", &test_int, 10, AF_INT_MIN, 100);
  cfg.bind_text("directive.of.text", &test_text, "default_value");
  cfg.bind_real("real_directive", &test_real, 666, AF_REAL_MIN, AF_REAL_MAX);
  cfg.bind_callback("directive_callback", directive_callback, args_to_cbk);
  cfg.bind_bool("mybool", &test_bool, false);

  printf("\n*** registered bindings ***\n");
  //cfg.print_bindings();

  unsigned long iter = 0;
  while (true) {
    printf("main loop: iteration #%lu\n", ++iter);
    cfg.print_bindings();
    printf("--\n");
    if (cfg.update()) {
      printf("\n*** config file modified ***\n");
      printf("the value of test_int is \033[1;32m%ld\033[m\n", test_int);
      printf("the value of test_text is \033[1;32m%s\033[m\n",
        test_text.c_str());
      printf("the value of test_real is \033[1;32m%lf\033[m\n", test_real);
      printf("the value of test_bool is \033[1;32m%s\033[m\n",
        test_bool ? "<true>" : "<false>");
    }

    if ((iter % 3) == 0) {
      bool unb = cfg.unbind("mybool");
      if (unb) printf("Unbound: mybool\n");
      else printf("Error unbinding mybool!\n");
    }

    sleep(1);
    if ((quit_requested) || (iter == iter_limits)) break;
  }

}

/** Test the operational queue facility.
 */
void test_queue() {

  unsigned int n_entries = 20;

  af::opQueue opq;
  opq.set_max_failures(2);  // after 2 errors mark as failed

  printf("\n=== INSERT ===\n");

  char urlbuf[300];
  for (unsigned int i=1; i<=n_entries; i++) {
    snprintf(urlbuf, 300, "root://www.google.it/num%09d/root_archive.zip#AliESDs.root", i);
    opq.cond_insert(urlbuf);
  }
  opq.dump();

  // Update
  for (unsigned int i=1; i<=3; i++) {
    opq.failed("root://www.google.it/num000000007/root_archive.zip#AliESDs.root");
    opq.failed("root://www.google.it/num000000011/root_archive.zip#AliESDs.root");
    printf("\n=== AFTER_FAILED_%u ===\n", i);
    opq.dump();
    opq.flush();
  }

  const af::queueEntry *ent;

  // Test three-step query
  std::cout << std::endl;
  opq.init_query_by_status(af::qstat_queue, 4);
  while ( ent = opq.next_query_by_status() ) {
    ent->print();
    std::cout << std::endl;
  }
  opq.free_query_by_status();

  printf("Marking a file as success\n");
  opq.success("root://www.google.it/num000000002/root_archive.zip#AliESDs.root");
  ent = opq.get_cond_entry("root://www.google.it/num000000002/root_archive.zip#AliESDs.root");
  ent->print();

  // Query a caso
  try {
    printf("\n=== ARBITRARY QUERY ===\n");
    //opq.arbitrary_query("SELECT COUNT(*) FROM queue WHERE main_url = 'prova' GROUP BY status");
    opq.arbitrary_query("SELECT COUNT(*),status FROM queue GROUP BY status");
    /*opq.arbitrary_query(
      "UPDATE queue SET status = CASE"
      "  WHEN (n_failures < 1) THEN 'S'"
      "  ELSE 'F'"
      "END");*/
    //opq.dump();
  }
  catch (std::exception &e) {
    printf("Arbitrary query failed: %s", e.what());
  }
  
  /*const af::queueEntry *qe =
    opq.get_entry("root://www.google.it/num000000001/root_archive.zip#AliESDs.root");

  if (qe) qe->print();
  else printf("entry not found\n");*/
}

/** Test regex facility.
 */
extern "C" void test_regex() {

  af::regex re;

  if (!re.set_regex_subst("^([^:]+)://(.+)$", "ftp://cern.ch/$2?proto=$1")) {
    printf("invalid regex");
  }
  else {
    const char *strings[] = {
      "http://www.google.it/",
      "root:///alice/sim/period/esd.root",
      "alien:///cern.ch/users/d/dberzano/esd.root",
      "not_an_url",
      ""
    };
    unsigned int sz = sizeof(strings)/sizeof(const char *);

    const char *res;

    for (unsigned int i=0; i<sz; i++) {
      res = re.subst(strings[i]);
      printf("%s --> %s\n", strings[i], res);
      //if (re.match(strings[i])) printf("[ OK ] %s\n", strings[i]);
      //else printf("[ NO ] %s\n", strings[i]);
    }

  }
}

/** Test the loading of dynamic libraries and symbols.
 */
void test_dl() {

  af::config cfg("/opt/afdsmgrd/trunk/dest/etc/afdsmgrd.conf");
  af::notify *notif = af::notify::load(
    "/opt/afdsmgrd/trunk/dest/lib/libafdsmgrd_notify_apmon.so", cfg);

  //cfg.print_bindings();

  if (!notif) {
    printf("Notification plugin not loaded!\n");
    return;
  }

  //
  // Plugin loaded henceforth
  //

  printf("Notification plugin loaded: \033[1;33m%s\033[m\n", notif->whoami());

  unsigned int iter = 0;
  unsigned int secs = 10;
  while (!quit_requested) {
    printf("Iteration #%u\n", iter++);

    if (cfg.update()) {
      printf("Configuration updated\n");
    }

    //
    // Notify datasets
    //

    // const char *ds_name, int n_files, int n_staged,
    // int n_corrupted, const char *tree_name, int n_events,
    // unsigned long long total_size_bytes
    notif->dataset(
      "/pippo/paperino/test",  // ds_name
      150,                     // n_files
      0,                       // n_corrupted
      147,                     // n_staged
      "/pippoTree",            // tree_name
      150000,                  // n_events
      123456789L               // total_size_bytes
    );

    printf("--\n");
    for (unsigned int elapsed_secs=0; elapsed_secs<secs; elapsed_secs++) {
      printf("Refreshing in %2u seconds...\r", secs-elapsed_secs);
      std::cout << std::flush;
      sleep(1);
      if (quit_requested) break;
    }
    printf("                           \r");
  }

  af::notify::unload(notif);

  printf("End of plugin test\n");

}

/** Tests the dollar substitution.
 */
void test_dollar_subst() {

  //const char *var_names[] = { "CIAO", "PIRLA", "_END_" };
  //const char *var_values[] = { "**ciao**", "**pirla**", "**begin**" };
  //unsigned int n_subst = 3;

  //std::string *s = af::regex::dollar_subst(
  //  "Ciao $PIRLA $PIRLA $PIRLA$PIRLA", n_subst,
  //  var_names, var_values);

  //if (s) {
  //  printf("AFT: <%s>\n", s->c_str());
  //  delete s;
  //}

  af::varmap_t variables;
  variables.insert( af::varpair_t( "CIAO", "**ciao**" ) );
  variables.insert( af::varpair_t( "PIRLA", "**pirla**" ) );
  variables.insert( af::varpair_t( "_END_", "**begin**" ) );

  std::string subst = "Ciao $NOPIRLA $CIAO $PIRLA$_END_";

  std::string s = af::regex::dollar_subst(subst.c_str(), variables);

  printf("BEF: <%s>\nAFT: <%s>\n", subst.c_str(), s.c_str());

}

/** Entry point.
 */
int main(int argc, char *argv[]) {

  signal(SIGTERM, signal_quit_callback);
  signal(SIGINT, signal_quit_callback);

  //test_regex();
  //test_dollar_subst();
  //test_dsmanip();
  test_queue();
  //test_extcmd(argv[0]);
  //test_config(999);
  //test_log();
  //test_dl();

  printf("!!! goodbye !!!\n");

  return 0;
}
