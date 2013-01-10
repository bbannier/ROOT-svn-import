/**
 * execwrapper.cc -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * Standalone program that acts as a helper for afdsmgrd to launch external
 * programs. It uses exec* functions to permit external programs to take the PID
 * of the current process, and before yielding the execution to the external
 * command it writes the current PID to an external file, and redirects stdout
 * and stderr.
 */
 
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Sets the specified environment variable.
 */
bool set_env_var(char *keyval) {
  char *val = strchr(keyval, '=');

  if (val) {
    *val = '\0';
    val++;
  }
  else val = (char *)"";

  if ( setenv(keyval, val, 1) == 0 ) return true;
  return false;
}

/** Entry point.
 */
int main(int argc, char *argv[]) {

  int c;

  opterr = 0;  // getopt lib: do not show standard errors

  const char *pid_fn = NULL;
  const char *out_fn = NULL;
  const char *err_fn = NULL;

  while ((c = getopt(argc, argv, "+:p:o:e:E:")) != -1) {
    switch (c) {
      case 'p':
        pid_fn = optarg;
      break;

      case 'o':
        out_fn = optarg;
      break;

      case 'e':
        err_fn = optarg;
      break;

      case 'E':
        if (!set_env_var(optarg)) {
          printf("Setting environment failed for: %s\n", optarg);
          return 9;
        }
      break;

      case ':':
        printf("Option '-%c' requires an argument\n", optopt);
        return 1;
      break;

      case '?':
        printf("Unknown option: '-%c'\n", optopt);
        return 2;
      break;
    }
  }

  // Check if output files were given
  bool missing_args = false;

  if (!pid_fn) {
    printf("PID file must be specified using -p <pidfile>\n");
    missing_args = true;
  }

  if (!out_fn) {
    printf("Standard output file must be specified using -o <outfile>\n");
    missing_args = true;
  }

  if (!err_fn) {
    printf("Standard error file must be specified using -e <errfile>\n");
    missing_args = true;
  }

  if (missing_args) {
    return 3;
  }

  if (optind == argc) {
    printf("Specify the command and its arguments to run after the other "
      "parameters\n");
    return 4;
  }

  // Fork to background
  pid_t current_pid = fork();
  if (current_pid > 0) {
    // Parent process: exit
    exit(0);
  }
  else if (current_pid < 0) {
    // Cannot fork: error
    printf("Fatal: cannot fork\n");
    exit(8);
  }

  // Assemble the command line
  std::ofstream pid_file(pid_fn);
  if (pid_file) {
    pid_file << getpid() << std::endl;
    pid_file.close();
  }
  else {
    printf("Can't write on PID file %s\n", pid_fn);
    return 5;
  }

  if (freopen(err_fn, "w", stderr) == NULL) {
    printf("Can't write standard error on file %s\n", err_fn);
    return 6;
  }

  if (freopen(out_fn, "w", stdout) == NULL) {
    printf("Can't write standard output on file %s\n", out_fn);
    return 7;
  }

  // Replace current process with external command
  execvp(argv[optind], &argv[optind]);

  // Never reached
  return 0;
}
