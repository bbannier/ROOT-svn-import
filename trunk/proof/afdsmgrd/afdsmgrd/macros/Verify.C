/**
 * Verify.C -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * ROOT macro to perform file verification. This macro is meant to be run by the
 * staging daemon and it should work with any protocol that does not have
 * peculiar prerequirements: this means that it is not limited to xrootd files.
 *
 * Taken arguments are:
 *
 *  - url      : the URL of the file to be inspected;
 *  - def_tree : the tree name to search for (either with or without leading /).
 *
 * If the default tree is not given, the first valid tree found in the file is
 * read. Trees stored in subdirectories are supported too.
 *
 * This macro works also as a non-compiled macro: just call it via:
 *
 *   root.exe -b -q Verify.C'("myproto://server:port//dir/file.zip#esd.root", \
 *     "/tree")'
 *
 * Remember to call root.exe and not just root to have afdsmgrd handling process
 * control correctly.
 *
 * In case of success, on stdout it returns a format like this:
 *
 * OK <orig_url_no_anchor> Size: <size_bytes> \
 *   EndpointUrl: <endpoint_url_w_anchor> Tree: <tree_name> Events: <n_events>
 *
 * Please note that "Tree:" reports the tree name with a leading /.
 *
 * If the file has been downloaded but no tree is present, the output is like:
 *
 * OK <orig_url_no_anchor> Size: <size_bytes> \
 *   EndpointUrl: <endpoint_url_w_anchor>
 *
 * In case of failure, on stdout it prints a format like this:
 *
 * FAIL <orig_url_no_anchor> <other_stuff...>
 *
 * Different reasons may lead to staging failures. If no reason is given, the
 * staging itself failed. Elsewhere, in <other_stuff...> there is a "Reason:"
 * string that may assume the following values:
 *
 *  - cant_open        : file can not be accessed
 *  - tree_disappeared : the tree exists in keys list but can't be read
 *  - no_such_tree     : the specified tree does not exist
 *
 * So, if the status is FAIL but we have a "Reason:", we conclude that the file
 * is staged but corrupted.
 *
 * In case of failure, if "Staged: 1" is reported, it means that the file has
 * been successfully staged, but it is corrupted. If no Staged: field is
 * reported, in case of failure the file is both unstaged and corrupted.
 */

/** Auxiliary function that finds a tree name given the specified TFile. If no
 *  default tree is found, an empty string is saved in def_tree. TTrees, or
 *  objects that inherit from TTree, are looked for only in the root TDirectory
 *  of the TFile.
 */
void DefaultTree(TFile *file, TString &def_tree) {

  TIter it(file->GetListOfKeys());
  TKey *key;
  Bool_t found = kFALSE;

  while (( key = dynamic_cast<TKey *>(it.Next()) )) {
    if (TClass::GetClass(key->GetClassName())->InheritsFrom("TTree")) {
      def_tree = key->GetName();  // without trailing slash
      found = kTRUE;
      break;
    }
  }

  if (!found) def_tree = "";

}

/** The main function of this ROOT macro.
 */
void Verify(const char *url, TString def_tree = "") {

  TUrl turl(url);
  TString anchor = turl.GetAnchor();

  turl.SetAnchor("");

  TFile *file = TFile::Open(url);  // open full file with anchor

  if (!file) {
    Printf("FAIL %s Staged: 1 Reason: cant_open", url);
    return;
  }

  // Get endpoint URL -- defaults to redirector URL
  const char *endp_url = url;
  const TUrl *endp_url_obj = file->GetEndpointUrl();
  if (endp_url_obj) {
    endp_url_obj->SetAnchor(anchor);
    endp_url = endp_url_obj->GetUrl();
  }

  if (def_tree == "") {
    // No default tree specified? Search for one in the root directory of TFile
    DefaultTree(file, def_tree);
  }
  else {
    // Normalize object path by removing double slashes and removint the
    // trailing one too
    Ssiz_t def_tree_prev_len;
    do {
      def_tree_prev_len = def_tree.Length();
      def_tree.ReplaceAll("//", "/");
    }
    while (def_tree_prev_len != def_tree.Length());
    if (def_tree.BeginsWith("/")) def_tree = def_tree(1,1e9);
  }

  // Search for the specified default tree
  if (def_tree != "") {

    TObject *obj;
    TTree   *tree;

    obj = file->Get(def_tree.Data());

    if (obj) {

      if (TClass::GetClass(obj->ClassName())->InheritsFrom("TTree")) {

        // All OK: object exists and it is a TTree (or inherits from it)
        tree = (TTree *)obj;
        Printf("OK %s Size: %lld EndpointUrl: %s Tree: /%s Events: %lld",
          turl.GetUrl(),      // without anchor (to mimic xrdstagetool)
          file->GetSize(),    // in bytes
          endp_url,           // with anchor
          def_tree.Data(),    // full path to TTree (/ prepended in fmt string)
          tree->GetEntries()  // number of events in tree
        );

      }
      else {

        // FAIL because object exists but it is not a TTree
        Printf("FAIL %s Size: %lld EndpointUrl: %s Staged: 1 "
          "Reason: not_a_tree", turl.GetUrl(), file->GetSize(), endp_url);

      }

    }
    else {

      // FAIL because the specified tree does not exist: this is a weak
      // indicator of file corruption. Since file has been staged, Staged=1
      Printf("FAIL %s Size: %lld EndpointUrl: %s Staged: 1 "
        "Reason: no_such_tree", turl.GetUrl(), file->GetSize(), endp_url);

    }

  }
  else {

    // OK but do not report number of events or tree name, because no tree has
    // been found
    Printf("OK %s Size: %lld EndpointUrl: %s", turl.GetUrl(), file->GetSize(),
      endp_url);

  }

  file->Close();
  delete file;

}
