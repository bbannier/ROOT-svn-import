/**
 * afDataSetList.cc -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * See header file for a description of the class.
 */

#include "afDataSetList.h"

using namespace af;

/** The only constructor for the class. It takes as an argument a pointer to the
 *  dataset manager used for the datasets requests.
 *
 *  The dataset manager is allowed to be NULL: in this case the member functions
 *  of this class will behave just like a dataset manager with no datasets
 *  inside.
 *
 *  Beware! This class owns the instance of TDataSetManagerFile!
 */
dataSetList::dataSetList(TDataSetManagerFile *_ds_mgr) :
  ds_mgr(_ds_mgr), ds_inited(false), fi_inited(false) {}

/** The destructor. It frees the memory taken by requests.
 */
dataSetList::~dataSetList() {
  set_dataset_mgr(NULL);
}

/** Frees the resources used by the former dataset manager and sets the new one.
 *
 *  The dataset manager is allowed to be NULL: see ctor's description for
 *  details.
 *
 *  Since the TDataSetManagerFile is owned by this class' instance, the former
 *  dataset manager, if not NULL, will be deleted first.
 */
void dataSetList::set_dataset_mgr(TDataSetManagerFile *_ds_mgr) {
  free_datasets();
  free_files();
  if (ds_mgr) delete ds_mgr;  // achtung!
  ds_mgr = _ds_mgr;
}

/** Initializes the list: this is the first function to call if you want to
 *  browse a list, and it is the function which actually performs the request to
 *  the dataset manager and takes memory.
 *
 *  All the memory allocations must be subsequently freed with the member
 *  free_datasets() function, or by deleting the instance.
 *
 *  This function is protected against erroneous double calls: if called a 2nd
 *  time, it does nothing.
 */
void dataSetList::fetch_datasets() {

  if ((ds_inited) || (ds_mgr == NULL)) return;

  TMap *groups = ds_mgr->GetDataSets("/*/*", TDataSetManager::kReadShort);

  groups->SetOwnerKeyValue();  // important to avoid leaks!
  TIter gi(groups);
  TObjString *gn;

  while ((gn = dynamic_cast<TObjString *>(gi.Next()))) {

    TMap *users = dynamic_cast<TMap *>( groups->GetValue( gn->String() ) );
    users->SetOwnerKeyValue();
    TIter ui(users);
    TObjString *un;

    while ((un = dynamic_cast<TObjString *>(ui.Next()))) {

      TMap *dss = dynamic_cast<TMap *>( users->GetValue( un->String() ) );
      dss->SetOwnerKeyValue();
      TIter di(dss);
      TObjString *dn;

      while ((dn = dynamic_cast<TObjString *>(di.Next()))) {

        // No COMMON user/group mapping is done here...
        TString dsUri = TDataSetManager::CreateUri( gn->String(),
          un->String(), dn->String() );

        ds_list.push_back( new std::string(dsUri.Data()) );

      }
    }
  }

  delete groups;

  ds_inited = true;
  ds_cur_idx = -1;
}

/** Frees the memory taken by previously querying the dataset manager for a
 *  datasets list. It is safe to call it even if no previous request has been
 *  performed.
 */
void dataSetList::free_datasets() {
  if (!ds_inited) return;
  unsigned int sz = ds_list.size();
  for (unsigned int i=0; i<sz; i++)
    delete ds_list[i];
  ds_list.clear();
  ds_inited = false;
}

/** Rewind the list pointer to the first element without re-performing the
 *  dataset request. It is safe to call it even if no previous request has been
 *  performed.
 */
void dataSetList::rewind_datasets() {
  if (!ds_inited) return;
  ds_cur_idx = -1;
}

/** Gets the next dataset name in the list. Returns NULL if list is not inited
 *  or if last element was reached. Elsewhere it returns a pointer to a buffer
 *  that contains the dataset name: the class owns buffer's memory which is
 *  overwritten by the next call of next(), so if you want to manipulate or
 *  store the dataset name you must make a copy of the returned buffer. Returned
 *  data is not ordered to avoid performance issues.
 */
const char *dataSetList::next_dataset() {
  if ((!ds_inited) || (++ds_cur_idx >= ds_list.size())) return NULL;
  return ds_list[ds_cur_idx]->c_str();
}

/** Gets the default tree name in datasets list. Returns NULL if no active
 *  dataset is found, or if no default tree name is set.
 */
const char *dataSetList::get_default_tree() {
  if (fi_coll) return fi_coll->GetDefaultTreeName();
  return NULL;
}

/** Sets the default tree name in datasets list. Returns false if no file
 *  collection is currently selected or if tree name did not change, true if
 *  default name has been changed.
 */
bool dataSetList::set_default_tree(const char *treename) {
  if ((fi_coll) && (treename)) {
    const char *old_treename = fi_coll->GetDefaultTreeName();
    if ((old_treename) && (strcmp(old_treename, treename) != 0)) {
      fi_coll->SetDefaultTreeName(treename);
      return true;
    }
  }
  return false;
}

/** Asks for the list of files (TFileInfo objs) for a given dataset name in
 *  current dataset manager. If ds_name is NULL then the last dataset name
 *  obtained via next_dataset() is used, if one. If reading of dataset fails for
 *  whichever reason, it returns false; if dataset was read successfully, it
 *  returns true. This function has to be called at the beginning of TFileInfos
 *  reading. It is safe to double call it - 2nd call does nothing (and returns
 *  true for success).
 *
 *  The filter argument tells the class to return only files that match some
 *  criteria when calling next_file(). Valid characters that represent criteria
 *  are:
 *
 *   - S (staged), s (not staged)
 *   - C (corrupted), c (not corrupted)
 *   - E (has number of events), e (hasn't number of events)
 *
 *  Criteria of the same type are combined with OR; different types are combined
 *  with AND. This means:
 *
 *   --> ((S || s) && (C || c) && (E || e))
 *
 *  If you don't specify, e.g., neither S nor s, it's like specifying *both* S
 *  and s. The same applies for Cc and Ee.
 */
bool dataSetList::fetch_files(const char *ds_name, const char *filter) {

  if (fi_inited) return true;
  if (!filter) return false;

  if (!ds_name) {
    if ((ds_inited) && (ds_cur_idx < ds_list.size()))
      ds_cur_name = *ds_list[ds_cur_idx];
    else return false;
  }
  else ds_cur_name = ds_name;

  fi_coll = ds_mgr->GetDataSet(ds_cur_name.c_str());
  if (!fi_coll) return false;

  fi_iter = new TIter(fi_coll->GetList());
  fi_inited = true;
  fi_curr = NULL;

  fi_filter.reset();

  if (strchr(filter, 'S')) fi_filter.set(idx_S);
  if (strchr(filter, 's')) fi_filter.set(idx_s);
  if (!fi_filter.test(idx_S) && !fi_filter.test(idx_s)) {
    fi_filter.set(idx_S);
    fi_filter.set(idx_s);
  }

  if (strchr(filter, 'C')) fi_filter.set(idx_C);
  if (strchr(filter, 'c')) fi_filter.set(idx_c);
  if (!fi_filter.test(idx_C) && !fi_filter.test(idx_c)) {
    fi_filter.set(idx_C);
    fi_filter.set(idx_c);
  }

  if (strchr(filter, 'E')) fi_filter.set(idx_E);
  if (strchr(filter, 'e')) fi_filter.set(idx_e);
  if (!fi_filter.test(idx_E) && !fi_filter.test(idx_e)) {
    fi_filter.set(idx_E);
    fi_filter.set(idx_e);
  }

  af::log::info(af::log_level_debug,
    "Filter on files: (S:%d || s:%d) && (C:%d || c:%d) && (E:%d || e:%d)",
    fi_filter.test(idx_S), fi_filter.test(idx_s),
    fi_filter.test(idx_C), fi_filter.test(idx_c),
    fi_filter.test(idx_E), fi_filter.test(idx_e));

  return true;
}

/** Frees the resources taken by the dataset list reading. This funcion must be
 *  called at the end of TFileInfos reading. It is safe to call it if
 *  fetch_files() has not been called yet: it does nothing in such a case.
 */
void dataSetList::free_files() {
  if (!fi_inited) return;
  delete fi_coll;
  delete fi_iter;
  fi_inited = false;
  fi_curr = NULL;
}

/** Resets the pointer so that the next call of next_file() will point to the
 *  first element in the list without the need to re-fetch it. It is safe to
 *  call it if fetch_files() has not been called yet: it does nothing in such a
 *  case.
 */
void dataSetList::rewind_files() {
  if (!fi_inited) return;
  fi_iter->Reset();
  fi_curr = NULL;
}

/** Returns a pointer to the next TFileInfo in the current dataset. If entry
 *  reading has not been prepared yet or if we reached the end of the list, NULL
 *  is returned instead.
 */
TFileInfo *dataSetList::next_file() {

  TFileInfoMeta *meta;
  bool s, c, e;

  if (!fi_inited) return NULL;

  while (true) {

    fi_curr = dynamic_cast<TFileInfo *>(fi_iter->Next());
    if (fi_curr == NULL) break;

    meta = fi_curr->GetMetaData(NULL);
    e = ((meta) && (meta->GetEntries() >=0));
    s = fi_curr->TestBit(TFileInfo::kStaged);
    c = fi_curr->TestBit(TFileInfo::kCorrupted);

    if (
      ((fi_filter.test(idx_S) && s) || (fi_filter.test(idx_s) && !s)) &&
      ((fi_filter.test(idx_C) && c) || (fi_filter.test(idx_c) && !c)) &&
      ((fi_filter.test(idx_E) && e) || (fi_filter.test(idx_e) && !e))
    ) break;

  }

  return fi_curr;  // may be NULL
}

/** Fetches the URL at the specified index from the currently selected
 *  TFileInfo. Index can either be from the beginning (if positive) or from the
 *  end (if negative); indexes start from 1 (or -1). If for any reason the URL
 *  can not be retireved (no TFileInfo selected, no such index...), NULL is
 *  returned.
 */
TUrl *dataSetList::get_url(int idx) {
  if (!fi_curr) return NULL;

  int nurls = fi_curr->GetNUrls();

  if (idx < 0) idx = nurls + 1 + idx;
  if ((idx <= 0) || (idx > nurls)) return NULL;

  TUrl *curl;

  fi_curr->ResetUrl();
  for (int i=1; i<=idx; i++) curl = fi_curr->NextUrl();

  fi_curr->ResetUrl();
  return curl;
}

/** Deletes all URLs except the last "howmany" (default to one) from the
 *  currently selected entry.
 *
 *  Returns an error code of type ds_manip_err_t (see) which indicates if there
 *  is an error or not, and in the latter case it indicates if data has been
 *  changed or not.
 */
ds_manip_err_t dataSetList::del_urls_but_last(unsigned int howmany) {

  if (!fi_curr) return ds_manip_err_fail;

  int nurls = fi_curr->GetNUrls();
  if ((nurls == 0) || (nurls == howmany)) return ds_manip_err_ok_noop;
  else nurls -= howmany;

  bool all_ok = true;

  TUrl *curl;

  fi_curr->ResetUrl();
  for (int i=0; i<nurls; i++) {
    curl = fi_curr->NextUrl();
    if (!curl) {
      all_ok = false;
      break;
    }
    urls_to_remove.push_back( new std::string(curl->GetUrl()) );
  }

  unsigned int sz = urls_to_remove.size();
  for (unsigned int i=0; i<sz; i++) {
    if (all_ok) {
      if ( !(fi_curr->RemoveUrl(urls_to_remove[i]->c_str())) ) {
        //af::log::error(af::log_level_debug, "Can't remove URL: %s",
        //  urls_to_remove[i]->c_str());
        all_ok = false;
      }
      //else {
      //  af::log::ok(af::log_level_debug, "Removed URL: %s",
      //    urls_to_remove[i]->c_str());
      //}
    }
    delete urls_to_remove[i];
  }

  urls_to_remove.clear();

  return (all_ok ? ds_manip_err_ok_mod : ds_manip_err_fail);
}

/** Tries to write the currently selected dataset on disk, if no parameters are
 *  given. Elsewhere, the given colleciton is written to the specified dataset
 *  URI. On success it returns true, false on failure.
 *
 *  TODO: directory creation on new datasets: WriteDataSet does not perform it!
 */
bool dataSetList::save_dataset(TFileCollection *fc, const char *ds_uri) {

  if ((!fc) || (!ds_uri)) {
    if (!fi_inited) return false;
    else {
      // Get currently selected dataset (through next_dataset())
      fc = fi_coll;
      ds_uri = ds_cur_name.c_str();
    }
  }

  TString group;
  TString user;
  TString name;
  ds_mgr->ParseUri( ds_uri, &group, &user, &name);

  fc->Update();
  int r = ds_mgr->WriteDataSet(group, user, name, fc);

  af::log::info(af::log_level_debug,
    "WriteDataSet(group=%s, user=%s, name=%s)=%d", group.Data(),
    user.Data(), name.Data(), r);

  if (r != 0) return true;

  //if (ds_mgr->RegisterDataSet(ds_uri, fc, "O") == 0)
  //  return true;

  return false;
}
