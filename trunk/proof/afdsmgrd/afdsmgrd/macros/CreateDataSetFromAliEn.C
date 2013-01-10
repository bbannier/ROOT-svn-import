/**
 * CreateDataSetFromAliEn.C -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * This macro is an interface to the afDataSetFromAliEn() function inside the
 * AFDSUtils package and allows the non-interactive creation of multiple
 * datasets directly from an AliEn find.
 *
 * Latest version of AFDSUtils package is automatically chosen.
 *
 * Instructions: http://aaf.cern.ch/node/160
 */

#include <TError.h>

void CreateDataSetFromAliEn(

  // Customize with your user and your group (optional)
  TString connStr   = "username:default@alice-caf.cern.ch",

  // Example for ESDs from official real data: the <RUN9> string in basePath
  // means to insert the run number, zero-padded to 9 digits. In dsPattern,
  // <RUN> is zero-padded to 9 digits by default. Keep in mind that basePath and
  // fileName are in fact the first two parameters of AliEn find, thus they
  // support basic jolly characters expansion features. The '*' is used as jolly
  // character. The filter option is used to subsequently match the AliEn search
  // with a custom extended regular expression. The anchor option is unused,
  // unless you specify "noautoarch" as option (see below)
  TString basePath  = "/alice/data/2010/LHC10h/<RUN9>/ESDs_lowflux/pass1/*.*",
  TString fileName  = "AliESDs.root",
  TString filter    = "",
  TString anchor    = "",
  TString treeName  = "/esdTree",
  TString runList   = "136837,137045",
  TString dsPattern = "LHC10h_<RUN>_ESDs_lowflux",

  // Example for ESDs from official Monte Carlo: run number is zero-padded to 6
  // digits
  /*
  TString basePath  = "/alice/sim/LHC10c9/<RUN6>",
  TString fileName  = "AliESDs.root",
  TString filter    = "",
  TString anchor    = "",
  TString treeName  = "/esdTree",
  TString runList   = "115315,119846",
  TString dsPattern = "MC_LHC10c9_<RUN>_ESDs",
  */

  // Example for all AODs from official real data: this example also shows how
  // to specify a run range
  /*
  TString basePath  = "/alice/data/2010/LHC10h/<RUN9>/ESDs/pass1",
  TString fileName  = "AliAOD.root",
  TString filter    = "",
  TString anchor    = "",
  TString treeName  = "/aodTree",
  TString runList   = "139104-139107,139306",
  TString dsPattern = "LHC10h_<RUN>_AllAODs",
  */

  // Host name (and, optional, port) of redirector -- leave empty "" if it is
  // the same as the host name in connStr, but it differs if you're connecting
  // to AAF through a SSH tunnel
  TString redirHost = "",
  //TString redirHost = "alice-caf.cern.ch",
 
  // Possible options: setstaged, cache, verify, commit, aliencmd, update,
  // noautoarch. By default each URL found is substituted with the URL of the
  // containing root_archive.zip pointing to the desired file: the "noautoarch"
  // option inhibits this substitution. Note that by default datasets are NOT
  // saved: you have to explicitly specify "commit"
  TString options   = "setstaged:aliencmd"

  ) {

  TProof::Open(connStr, "masteronly");
  if (!gProof) {
    ::Error(gSystem->HostName(), "Problem connecting to PROOF, aborting");
    return;
  }

  if (EnableLatestPackage("VO_ALICE@AFDSUtils")) {
    ::Error(gSystem->HostName(), "Can't enable AFDSUtils package, aborting");
    return;
  }

  TString afHost = connStr.Remove(0, connStr.Index('@')+1);
  {
    Ssiz_t colon = afHost.Index(':');
    if (colon > -1) afHost.Remove(colon, afHost.Length());
  }

  afSetDsPath("/pool/PROOF-AAF/proof/dataset");
  afSetProofUserHost(connStr.Data());
  afSetProofMode(1);
  afSetRedirUrl( Form("root://%s/$1",
    redirHost.IsNull() ? afHost.Data() : redirHost.Data()) );

  afPrintSettings();

  afDataSetFromAliEn(basePath, fileName, filter, anchor, treeName, runList,
    dsPattern, options);

}

/** Auxiliary function to enable latest version of AFDSUtils package. Return
 *  values are the same as TProof::EnablePackage(): -1 on error, 0 on success.
 */
Int_t EnableLatestPackage(TString packageName, Bool_t notOnClient = kFALSE) {

  if (!gProof) return -1;

  TString tmpFile = "/tmp/EnableLatestProofPackage.txt";

  gSystem->Unlink(tmpFile.Data());
  gSystem->RedirectOutput(tmpFile.Data());
  gProof->ShowPackages();
  gSystem->RedirectOutput(0x0);

  char buf[500];
  ifstream fp(tmpFile.Data());

  if (!fp) {
    ::Error("EnableLatestPackage", "Can't open temporary file: %s",
      tmpFile.Data());
    return -1;
  }

  TString reStr = Form("%s::(([0-9])\.([0-9])\.([0-9])(-([^ ]+))?)\.par",
    packageName.Data());
  TPMERegexp re = reStr;

  Int_t curMaj = -1;
  Int_t curMin = -1;
  Int_t curPat = -1;
  TString curPackage = "";

  while ( fp.getline(buf, 1000)) {
    Int_t nMatch = re.Match(buf);
    if (nMatch >= 5) {
      Int_t maj = re[3].Atoi();
      Int_t min = re[4].Atoi();
      Int_t pat = re[5].Atoi();

      Bool_t newMax = kFALSE;

      if (maj > curMaj) newMax = kTRUE;
      else if (maj == curMaj) {
        if (min > curMin) newMax = kTRUE;
        else if ((min == curMin) && (pat > curPat)) newMax = kTRUE;
      }

      if (newMax) {
        curMaj = maj;
        curMin = min;
        curPat = pat;
        curPackage = re[0];
      }
    }
  }

  fp.close();

  gSystem->Unlink(tmpFile.Data());

  if (curPackage.IsNull()) {
    ::Error("EnableLatestPackage", "Can't find package: %s",
      packageName.Data());
    return -1;
  }
  else {
    ::Info("EnableLatestPackage", "Most recent version of package: %s",
      curPackage.Data());
    return gProof->EnablePackage(curPackage.Data());
  }

}
