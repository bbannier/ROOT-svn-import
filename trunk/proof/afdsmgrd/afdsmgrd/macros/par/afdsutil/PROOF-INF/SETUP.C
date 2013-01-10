/**
 * SETUP.C -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * Setup macro for PARfile of AFDSUtil. This PARfile contains the afdsutil.C
 * macro only, which is loaded and built by this setup macro when needed only
 * on the client.
 */

void SETUP() {
  if ((gProof) && (gProof->TestBit(TProof::kIsClient))) {
    // We are on client
    gROOT->LoadMacro("afdsutil.C+");
    afdsutil();
  }
}
