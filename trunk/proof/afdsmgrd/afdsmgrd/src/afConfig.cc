/**
 * afConfig.cc -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * See header file for a description of the class.
 */

#include "afConfig.h"

using namespace af;

////////////////////////////////////////////////////////////////////////////////
// Member functions for the af::cfg_binding class
////////////////////////////////////////////////////////////////////////////////

/** Allowed values for "true" and "false" booleans -- notice the empty string
 *  inside the true list, meaning that the sole presence of the directive
 *  without any value means that the boolean will be set to true.
 */
const char *cfg_binding::true_str[] = { "1", "true", "t", "yes", "y", "" };
const char *cfg_binding::false_str[] = { "0", "false", "f", "no", "n" };
size_t cfg_binding::true_str_len = sizeof(true_str)/sizeof(const char *);
size_t cfg_binding::false_str_len = sizeof(false_str)/sizeof(const char *);

/** Constructor of the directive binding for booleal type.
 */
cfg_binding::cfg_binding(const char *name, bool *_dest_ptr, bool _def_val) :
  dir_name(name), dest(_dest_ptr), type(dir_type_bool) {
  def_val.b = _def_val;
}

/** Constructor of the directive binding for numeric type "integer".
 */
cfg_binding::cfg_binding(const char *name, long *_dest_ptr, long _def_val,
  long _min, long _max) :
  dir_name(name), dest(_dest_ptr), type(dir_type_int) {
  ctor_helper(_def_val, _min, _max);
}

/** Constructor of the directive binding for numeric type "unsigned integer".
 */
cfg_binding::cfg_binding(const char *name, unsigned long *_dest_ptr,
  unsigned long _def_val, unsigned long _min, unsigned long _max) :
  dir_name(name), dest(_dest_ptr), type(dir_type_uint) {
  ctor_helper(_def_val, _min, _max);
}

/** Constructor of the directive binding for numeric type "real".
 */
cfg_binding::cfg_binding(const char *name, double *_dest_ptr, double _def_val,
  double _min, double _max) :
  dir_name(name), dest(_dest_ptr), type(dir_type_real) {
  ctor_helper(_def_val, _min, _max);
}

/** Constructor of the directive binding for text type. Default value is copied
 *  in an internal buffer (destroyed on exit).
 */
cfg_binding::cfg_binding(const char *name, std::string *_dest_ptr,
  const char *_def_val) :
  dir_name(name), dest(_dest_ptr), type(dir_type_text) {
  def_val.s = new std::string(_def_val);
}

/** Constructor of the directive binding for a custom type handled by an
 *  external callback.
 */
cfg_binding::cfg_binding(const char *name,
  void (callback)(const char *name, const char *val, void *args), void *args) :
  dir_name(name), type(dir_type_custom), ext_callback(callback),
  callback_args(args), prev_val(NULL), first_callback(true) {}

/** Destructor. It deletes some string pointers, if the directive type is
 *  appropriate.
 */
cfg_binding::~cfg_binding() {
  if (type == dir_type_text) delete def_val.s;
  else if ((type == dir_type_custom) && (prev_val)) delete prev_val;
}

/** Constructor helper (templatized).
 */
template<typename T> void cfg_binding::ctor_helper(T _def_val, T _min, T _max) {
  *(T *)(&min) = _min;
  *(T *)(&max) = _max;
  *(T *)(&def_val) = _def_val;
  touched = false;
}

/** Prints on stdout the directive type and the pointer.
 */
void cfg_binding::print() const {
  printf("name:%s type:", dir_name.c_str());
  switch (type) {
    case dir_type_int:    printf("int");  break;
    case dir_type_uint:   printf("uint"); break;
    case dir_type_real:   printf("real"); break;
    case dir_type_text:   printf("text"); break;
    case dir_type_custom: printf("cstm"); break;
  }

  if (type == dir_type_custom) {
    printf(" fnc:0x%016lx\n", (unsigned long)ext_callback);
  }
  else {
    printf(" ptr:0x%016lx\n", (unsigned long)dest);
  }
}

/** Assigns a value, parsing it to the appropriate directive type, and checking
 *  the limits. If the value does not fit limits, it is set to the default
 *  value. Default value is not checked agains limits: this is done on purpose
 *  to permit error detection through the assignment of a default directive. The
 *  variable val_str can be transitional and the memory it takes can be freed
 *  right after calling assign().
 */
void cfg_binding::assign(const char *val_str) {

  mixed_t val;
  int i;

  switch (type) {

    ////////////////////////////////////////////////////////////////////////////
    // Boolean
    ////////////////////////////////////////////////////////////////////////////
    case dir_type_bool:

      for (i=0; i<true_str_len; i++) {
        if (strcmp(val_str, true_str[i]) == 0) {
          i = -1;
          val.b = true;
          break;
        }
      }
      if (i != -1) {
        for (i=0; i<false_str_len; i++) {
          if (strcmp(val_str, false_str[i]) == 0) {
            i = -1;
            val.b = false;
            break;
          }
        }
      }

      *(bool *)dest = (i == -1) ? val.b : def_val.b;

    break;

    ////////////////////////////////////////////////////////////////////////////
    // Integer
    ////////////////////////////////////////////////////////////////////////////
    case dir_type_int:

      val.i = strtol(val_str, NULL, 0);
      *(long *)dest = fits_limits(val.i, min.i, max.i) ? val.i : def_val.i;

    break;

    ////////////////////////////////////////////////////////////////////////////
    // Unsigned integer
    ////////////////////////////////////////////////////////////////////////////
    case dir_type_uint:

      val.u = strtoul(val_str, NULL, 0);
      *(unsigned long *)dest = fits_limits(val.u, min.u, max.u) ?
        val.u : def_val.u;

    break;

    ////////////////////////////////////////////////////////////////////////////
    // Real
    ////////////////////////////////////////////////////////////////////////////
    case dir_type_real:

      val.r = strtod(val_str, NULL);
      *(double *)dest = fits_limits(val.r, min.r, max.r) ? val.r : def_val.r;

    break;

    ////////////////////////////////////////////////////////////////////////////
    // Text
    ////////////////////////////////////////////////////////////////////////////
    case dir_type_text:

      if (*(std::string *)dest != val_str) *(std::string *)dest = val_str;

    break;

    ////////////////////////////////////////////////////////////////////////////
    // Custom type: invoke callback
    ////////////////////////////////////////////////////////////////////////////
    case dir_type_custom:

      if (prev_val == NULL) {
        prev_val = new std::string(val_str);
        ext_callback(dir_name.c_str(), val_str, callback_args);
      }
      else if (*prev_val != val_str) {
        *prev_val = val_str;
        ext_callback(dir_name.c_str(), val_str, callback_args);
      }

      if (first_callback) first_callback = false;

    break;

  }
}

/** Assigns the default value for the current binding. No limits check for the
 *  default value is performed on purpose, to eventually permit error signalling
 *  through the assignment of a default value.
 */
void cfg_binding::assign_default() {

  switch (type) {

    case dir_type_bool:
      *(bool *)dest = def_val.b;
    break;

    case dir_type_int:
      *(long *)dest = def_val.i;
    break;

    case dir_type_uint:
      *(unsigned long *)dest = def_val.u;
    break;

    case dir_type_real:
      *(double *)dest = def_val.r;
    break;

    case dir_type_text:
      if ( *(std::string *)dest != *(def_val.s) )
        *(std::string *)dest = *def_val.s;
    break;

    case dir_type_custom:
      if (prev_val) {
        delete prev_val;
        prev_val = NULL;
        ext_callback(dir_name.c_str(), NULL, callback_args);
      }
      else if (first_callback) {
        first_callback = false;
        ext_callback(dir_name.c_str(), NULL, callback_args);
      }

    break;

  }

}

////////////////////////////////////////////////////////////////////////////////
// Member functions for the af::config class
////////////////////////////////////////////////////////////////////////////////

/** Constructor.
 */
config::config(const char *config_file) :
  file_name(config_file), never_read_before(true) {
  memset(&file_stat, 0, sizeof(struct stat));
}

/** Destructor. It solely calls the function that unbinds every directive.
 */
config::~config() {
  unbind_all();
}

/** Prints fields (used for debug and tests).
 */
void config::print_bindings() {
  for (conf_dirs_iter_t it=directives.begin(); it!=directives.end(); it++)
    (*it)->print();
}

/** Binds a "boolean" directive to a pointer.
 */
void config::bind_bool(const char *dir_name, bool *dest_ptr, bool def_val) {
  cfg_binding *binding = new cfg_binding(dir_name, dest_ptr, def_val);
  directives.push_back(binding);
}

/** Binds an "integer" directive to a pointer.
 */
void config::bind_int(const char *dir_name, long *dest_ptr, long def_val,
  long min_eq, long max_eq) {
  cfg_binding *binding = new cfg_binding(dir_name, dest_ptr, def_val, min_eq,
    max_eq);
  directives.push_back(binding);
}

/** Binds an "unsigned integer" directive to a pointer.
 */
void config::bind_uint(const char *dir_name, unsigned long *dest_ptr,
  unsigned long def_val, unsigned long min_eq, unsigned long max_eq) {
  cfg_binding *binding = new cfg_binding(dir_name, dest_ptr, def_val, min_eq,
    max_eq);
  directives.push_back(binding);
}

/** Binds a "real" directive to a pointer.
 */
void config::bind_real(const char *dir_name, double *dest_ptr, double def_val,
  double min_eq, double max_eq) {
  cfg_binding *binding = new cfg_binding(dir_name, dest_ptr, def_val, min_eq,
    max_eq);
  directives.push_back(binding);
}

/** Binds a "text" directive to a STL string pointer.
 */
void config::bind_text(const char *dir_name, std::string *dest_ptr,
  const char *def_val) {
  cfg_binding *binding = new cfg_binding(dir_name, dest_ptr, def_val);
  directives.push_back(binding);
}

/** Binds a generic directive whose control (limits check, assignment, pointer
 *  validity, whatever) is taken by a custom callback function. This callback
 *  function must support a NULL val pointer meaning that no such directive
 *  exists in configuration file, and the default value should be used instead.
 */
void config::bind_callback(const char *dir_name,
  void (callback)(const char *name, const char *val, void *args), void *args) {
  cfg_binding *binding = new cfg_binding(dir_name, callback, args);
  directives.push_back(binding);
}

/** Returns a pointer to the first non-blank char of the input string. The
 *  original string is not modified.
 */
char *config::ltrim(char *str) const {
  char *tr;
  for (tr=str; *tr!='\0'; tr++) {
    if ((*tr != '\t') && (*tr != ' ')) break;
  }
  return tr;
}

/** Truncates the string at the first blank character. The original string is
 *  modified.
 */
char *config::rtrim(char *str) const {
  char *p;
  for (p=str; *p!='\0'; p++) {}
  for (p--; p>=str; p--) {
    if ((*p == '\t') || (*p == ' ')) *p = '\0';
    else break;
  }
  return str;
}

/** Reads configuration from file. Only directives bound to a value are parsed,
 *  the others are ignored. Instead, all the variables (the "set" directives)
 *  are read and stored in an internal vector.
 */
void config::read_file() {

  std::ifstream file(file_name.c_str());

  if (!file) return;
  else {
    if (stat(file_name.c_str(), &file_stat) != 0) {
      // File disappeared after opening
      file.close();
      return;
    }
  }

  never_read_before = false;

  varmap_t variables;

  char *dir;  // the full directive, trimmed (part of strbuf)
  char *val;  // the string value of the directive, trimmed (part of strbuf)
  char *var;

  while ( file.getline(strbuf, AF_CONFIG_BUFSIZE) ) {

    dir = ltrim(strbuf);
    if ((*dir == '#') || (*dir == '\0')) continue;

    for (val=dir; *val!='\0'; val++) {
      if ((*val == '\t') || (*val == ' ')) {
        *val++ = '\0';
        break;
      }
    }

    // Check if it is a variable in the form:
    // set VARIABLE=value of variable
    // Variable name and value are trimmed

    if (strncmp("set", dir, 3) == 0) {

      // Variable: check if valid, then store value in a vector

      var = rtrim(ltrim(val));
      bool search_for_equal = false;
      bool equal_found = false;

      for (val=var; *val!='\0'; val++) {

        if (search_for_equal) {

          if (*val == '=') {
            *val++ = '\0';
            equal_found = true;
            break;
          }
          else if ((*val != ' ') && (*val != '\t')) break;  // invalid

        }
        else {

          if ((*val == ' ') || (*val == '\t')) {
            *val = '\0';
            search_for_equal = true;
          }
          else if (*val == '=') {
            *val++ = '\0';
            equal_found = true;
            break;
          }
          else if (((*val < '0') || (*val > '9')) &&
            ((*val < 'a') || (*val > 'z')) &&
            ((*val < 'A') || (*val > 'Z')) && (*val != '_')) break;  // invalid

        }

      }

      if (equal_found) {
        val = ltrim(val);
        variables.insert( varpair_t(var, val) );
      }

    }
    else {

      // Directive: check if it has a binding, and substitute variables

      for (conf_dirs_iter_t it=directives.begin(); it!=directives.end(); it++) {
       cfg_binding &binding = **it;
        if (strcmp(binding.get_name(), dir) == 0) {
          val = rtrim(ltrim(val));

          if (variables.size() > 0) {
            std::string val_subst = regex::dollar_subst(val, variables);
            binding.assign(val_subst.c_str());
          }
          else {
            binding.assign(val);
          }

          binding.touch();
          break;
        }
      }

    }

  }

  // Print out all variables found, and their respective values
  /*for ( varmap_const_iter_t it=variables.begin(); it!=variables.end(); it++) {
    printf("*** {%s}={%s} ***\n",
      it->first.c_str(),
      it->second.c_str());
  }*/


  file.close();

  // At this point, bound and unassigned directives should be defaulted: see
  // the untouched directives

  for (conf_dirs_iter_t it=directives.begin(); it!=directives.end(); it++) {
   cfg_binding &binding = **it;
   if (!binding.is_touched()) binding.assign_default();
   else binding.touch(false);
  }

}

/** Checks if a file has been updated; in such a case, re-read it. If file is
 *  not readable, returns false and does not modify current variables. If file
 *  is readable it returns true if it has been modified since last access, false
 *  elsewhere.
 */
bool config::update() {

  struct stat upd_file_stat;
  memset(&upd_file_stat, 0, sizeof(struct stat));

  if (stat(file_name.c_str(), &upd_file_stat) != 0) {
    if (never_read_before) {
      never_read_before = false;
      default_all();
      return true;
    }
    else return false;
  }

  bool modified = false;

  if ((upd_file_stat.st_size != file_stat.st_size) ||
    (upd_file_stat.st_mtime != file_stat.st_mtime)) {
    modified = true;
    read_file();
  }

  return modified;
}

/** Assigns default values to every directive.
 */
void config::default_all() {
  for (conf_dirs_iter_t it=directives.begin(); it!=directives.end(); it++)
    (*it)->assign_default();
}

/** Removes a single binding by invoking its destructor: returns true on
 *  success, false if that binding did not exist.
 */
bool config::unbind(const char *dir_name) {

  for (conf_dirs_iter_t it=directives.begin(); it!=directives.end(); it++) {
    if ( strcmp(dir_name, (*it)->get_name()) == 0 ) {
      delete *it;
      directives.erase(it);
      return true;
    }
  }

  return false;
}

/** This function unbinds every directive: it destructs every pointer to
 *  cfg_binding and empties the lsit.
 */
void config::unbind_all() {
  for (conf_dirs_iter_t it=directives.begin(); it!=directives.end(); it++)
    delete *it;
  directives.clear();
}
