/**
 * afRegex.cc -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * See header file for a description of the class.
 */

#include "afRegex.h"

using namespace af;

/** Constructor.
 */
regex::regex() : re_match(NULL), re_subst(NULL), sub_ptn(NULL) {}

/** Destructor.
 */
regex::~regex() {
  unset_regex_match();
  unset_regex_subst();
}

/** Sets and compiles the extended regular expression used to match strings. If
 *  there is an error in the regex it returns false and the former match regex
 *  is left intact; if everything went right, true is returned.
 */
bool regex::set_regex_match(const char *ptn) {

  if (ptn == NULL) return false;

  regex_t *re_compd = new regex_t;

  // See: http://www.gnu.org/s/libc/manual/html_node/
  // Flags-for-POSIX-Regexps.html#Flags-for-POSIX-Regexps
  if (regcomp(re_compd, ptn, REG_EXTENDED|REG_NOSUB) != 0) {
    delete re_compd;
    return false;
  }
  else {
    unset_regex_match();
    re_match = re_compd;
  }

  return true;
}

/** Frees the space used by the match regex. Safe to call if match regex was not
 *  previously set (it does nothing in such a case).
 */
void regex::unset_regex_match() {
  if (re_match) {
    regfree(re_match);
    delete re_match;
    re_match = NULL;
  }
}

/** Matches the given string to the previously set regular expression. If match
 *  succeeds it returns true; false if it fails. If the matching regex is not
 *  set, it always matches. If the given string is NULL, false is returned.
 */
bool regex::match(const char *str) {

    if (str == NULL) return false;
    if (re_match == NULL) return true;

    // See: http://www.gnu.org/s/libc/manual/html_node/
    // Matching-POSIX-Regexps.html#Matching-POSIX-Regexps
    int exec_status = regexec(re_match, str, 0, NULL, 0);

    if (exec_status == 0) return true;
    else if (exec_status != REG_NOMATCH)
      throw new std::runtime_error("Not enough memory to perform regex!");

    return false;
}

/** Sets and compiles the extended regular expression used for substitutions. If
 *  there is an error in the regex it returns false and the former substitution
 *  regex is left intact; if everything went right, true is returned.
 */
bool regex::set_regex_subst(const char *ptn, const char *_sub_ptn) {

  if ((_sub_ptn == NULL) || (ptn == NULL)) return false;

  regex_t *re_compd = new regex_t;

  // See: http://www.gnu.org/s/libc/manual/html_node/
  // Flags-for-POSIX-Regexps.html#Flags-for-POSIX-Regexps
  if (regcomp(re_compd, ptn, REG_EXTENDED) != 0) {
    delete re_compd;
    return false;
  }
  else {
    unset_regex_subst();
    re_subst = re_compd;
  }

  // Prepare substitution pattern

  sub_ptn = strdup(_sub_ptn);
  if (sub_ptn == NULL)
    throw new std::runtime_error("Not enough memory to store pattern");

  submatch_t cur_part;
  cur_part.index = 0;
  cur_part.ptr = sub_ptn;
  sub_parts.push_back(cur_part);

  for (char *ptr = sub_ptn; *ptr!='\0'; ptr++) {
    if (ptr[0] == '$') {
      if ((ptr[1] >= '0') && (ptr[1] <= '9')) {
        cur_part.index = ptr[1] - '0';
        cur_part.ptr = &ptr[2];
        sub_parts.push_back(cur_part);
        *ptr = '\0';
        ptr++;
      }
    }
  }

  return true;
}

/** Frees the space used by the substitution regex and substitution pattern.
 *  Safe to call if substitution regex was not previously set (it does nothing
 *  in such a case).
 */
void regex::unset_regex_subst() {
  if (re_subst) {
    regfree(re_subst);
    delete re_subst;
    re_subst = NULL;
    free(sub_ptn);
    sub_ptn = NULL;
    sub_parts.clear();
  }
}

/** Builds a new string (returned) from the original string (passed) based on
 *  the previously entered substitution regex and pattern. If no substitution
 *  regex was given or the original string is NULL, returns NULL.
 *
 *  This function returns a pointer to an internal static buffer: copy it
 *  before reusing member functions of this class again.
 *
 *  If the original string does not match the regular expression, NULL is
 *  returned too.
 */
const char *regex::subst(const char *orig_str) {

  if ((re_subst == NULL) || (orig_str == NULL)) return NULL;

  // Match
  static const int n_match = 10;
  regmatch_t match[n_match];

  // See: http://www.gnu.org/s/libc/manual/html_node/
  // Matching-POSIX-Regexps.html#Matching-POSIX-Regexps
  if (regexec(re_subst, orig_str, n_match, match, 0) != 0) return NULL;

  // Substitute: assemble stuff (memory safe)
  strncpy(strbuf, sub_parts[0].ptr, AF_REGEX_BUFSIZE);
  size_t left = AF_REGEX_BUFSIZE-strlen(strbuf);
  for (unsigned int i=1; i<sub_parts.size(); i++) {

    if (sub_parts[i].index >= n_match) {
      strncat(strbuf, "<?>", left);
      left -= 3;
    }
    else {
      regmatch_t *m = &match[sub_parts[i].index];
      size_t part_len;
      if (m->rm_eo >= 0) {
        part_len = m->rm_eo-m->rm_so;
        if (part_len >= left) break;
        strncat(strbuf, &orig_str[m->rm_so], part_len);
        left -= part_len;
      }
    }

    strncat(strbuf, sub_parts[i].ptr, left);
  }

  return strbuf;
}

/** Dollar substitution for variables in format $VARIABLE. Variable names may
 *  contain only 0-9, A-Z, a-z, and underscore. This is a static function.
 *
 *  A std::string is returned containing the original string with variables
 *  substituted; variables which weren't found are left intact. If the input
 *  pattern is a NULL pointer, an empty string is returned.
 *
 *  Key/value pairs ought to be passed through a std::map object aliased to
 *  type varmap_t (see header for more information on typedefs).
 */
std::string regex::dollar_subst(const char *ptn, varmap_t &variables) {

  std::string output_string;

  if (!ptn) return output_string;

  bool in_variable = false;
  const char *beg_var = NULL;
  static char this_var_name[100];
  int offset = 0;

  output_string = ptn;

  // No substitution requested
  if (variables.size() == 0) return output_string;

  for (const char *p=ptn; *p!='\0'; p++) {

    if (in_variable) {

      bool close_variable = false;

      if (((*p < '0') || (*p > '9')) && ((*p < 'a') || (*p > 'z')) &&
        ((*p < 'A') || (*p > 'Z')) && (*p != '_')) {

        // Terminating character found
        close_variable = true;
      }
      else if (*(p+1) == '\0') {

        // This one is a valid character, but the last one of input string
        p++;
        close_variable = true;

      }

      // Do we need to close the variable?
      if (close_variable) {
        in_variable = false;
        beg_var++;
        unsigned int var_name_len = p - beg_var;

        if (var_name_len == 0) continue;  // only '$'? Skip!
        else if (var_name_len >= 100) var_name_len = 99;  // limited buffer

        strncpy(this_var_name, beg_var, var_name_len);
        this_var_name[var_name_len] = '\0';

        // Search for variable value
        varmap_const_iter_t it = variables.find(this_var_name);

        if (it != variables.end()) {

          const std::string &this_var_val = it->second;

          // Replace $VARIABLE with its value, if value was found
          unsigned int sub_pos = beg_var-ptn - 1 + offset;
          unsigned int sub_len = var_name_len + 1;
          output_string.replace(sub_pos, sub_len, this_var_val);
          offset += (this_var_val.length() - var_name_len - 1);
 
        }

        // Maybe we incremented p once more because of imminent end of string:
        // let's break the loop here to avoid trespassing it!
        if (*p == '\0') break;

      }

    }

    // (Re)open the variable on dollar
    if (*p == '$') {
      // Begin of a variable name
      in_variable = true;
      beg_var = p;
    }

  }

  return output_string;
}
