/**
 * LocateVerifyXrd.C -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * This macro is meant to be called by the verifier, not the daemon!
 *
 * Given a file's xrootd redirector URL, it checks:
 *
 *  - whether the file is *really* staged or not;
 *  - its endpoint URL.
 *
 * This macro works also as a non-compiled macro: just call it via:
 *
 *   root.exe -b -q \
 *     LocateVerifyXrd.C'("root://server:port//dir/file.zip#esd.root")
 *
 * Remember to call root.exe and not just root to have the calling program
 * handling process control correctly.
 *
 * - CASE 1: file is really staged and can be found on some server:
 *     OK <orig_url_no_anchor> EndpointUrl: <endpoint_url_w_anchor>
 *
 * - CASE 2: file is not staged: no EndpointUrl is reported (don't be mistaken
 *   by the "OK", it just means that the verification succeeded even if the file
 *   is not there):
 *     FAIL <orig_url_no_anchor> Reason: not_staged
 *
 * - CASE 3: can't initialize the stager interface: no reason is reported
 *     FAIL <orig_url_no_anchor>
 *
 * - CASE 4: file is corrupted: the reason why is reported
 *     FAIL <orig_url_no_anchor> Reason: <a_reason>
 */

/** Auxiliary function that finds a tree name given the specified TFile. If no
 *  default tree is found, an empty string is saved in def_tree.
 */
void DefaultTree(TFile *file, TString &def_tree) {

  TIter it(file->GetListOfKeys());
  TKey *key;
  Bool_t found = kFALSE;

  while (( key = dynamic_cast<TKey *>(it.Next()) )) {
    if (TClass::GetClass(key->GetClassName())->InheritsFrom("TTree")) {
      def_tree = key->GetName();  // without initial slash
      found = kTRUE;
      break;
    }
  }

  if (!found) def_tree = "";

}

/** Main function.
 */
void LocateVerifyXrd(const char *redir_url, TString def_tree = "",
  Bool_t deep = kFALSE) {

  TUrl turl(redir_url);
  TString turl_anchor = turl.GetAnchor();
  turl.SetAnchor("");

  TUrl stager_url( Form("%s://%s:%d", turl.GetProtocol(), turl.GetHost(),
    turl.GetPort()) );

  TFileStager *stager = TFileStager::Open(stager_url.GetUrl());

  if ((!stager) || (!stager->IsValid())) {
    Printf("FAIL %s", turl.GetUrl());
    return;
  }

  //
  // Locate file
  //

  TString endp_url;
  Int_t r = stager->Locate(redir_url, endp_url);

  if (r == 0) {
    // Staged: do deep verify if requested
    if (deep) {

      TFile *file = TFile::Open(redir_url);  // open full file with anchor

      if (!file) {
        Printf("FAIL %s EndpointUrl: %s Reason: cant_open", url,
          endp_url.Data());
        return;
      }

      // Remove initial slash
      if (def_tree.BeginsWith("/")) def_tree = def_tree(1,1e9);

      // No default tree specified? Search for one
      if (def_tree == "") DefaultTree(file, def_tree);

      // Search for the specified default tree
      if (def_tree != "") {

        TKey *key = file->FindKey(def_tree.Data());
        if (key &&
          TClass::GetClass(key->GetClassName())->InheritsFrom("TTree")) {

          TTree *tree = dynamic_cast<TTree *>(key->ReadObj());
          if (tree) {

            // All OK
            Printf("OK %s Size: %lld EndpointUrl: %s Tree: /%s Events: %lld",
              turl.GetUrl(),      // without anchor (mimic xrdstagetool)
              file->GetSize(),    // in bytes
              endp_url.Data(),    // with anchor
              def_tree.Data(),    // tree name (init. slash is added in fmt str)
              tree->GetEntries()  // events in tree
            );

          }
          else {
            // FAIL because the specified tree disappeared from key: do not
            // report tree name and events. This should not happen and it is a
            // strong indicator of file corruption
            Printf("FAIL %s Size: %lu EndpointUrl: %s Reason: tree_disappeared",
              turl.GetUrl(), size, endp_url.Data());
          }

        }
        else {
          // FAIL because the specified tree does not exist: this is a weak
          // indicator of file corruption. Since file has been staged, Staged=1
          Printf("FAIL %s Size: %lu EndpointUrl: %s Reason: no_such_tree",
            turl.GetUrl(), size, endp_url.Data());
        }

      }
      else {

        // OK but do not report number of events or tree name, because no tree
        // has been found
        Printf("OK %s Size: %lu EndpointUrl: %s", turl.GetUrl(), size,
          endp_url.Data());

      }

      file->Close();
      delete file;

    }
    else {
      // Shallow verification
      Printf("OK %s EndpointUrl: %s", redir_url, endp_url.Data());
    }
  }
  else {
    // Not staged: this condition is treated as a failure
    Printf("FAIL %s Reason: not_staged", redir_url);
  }

}
