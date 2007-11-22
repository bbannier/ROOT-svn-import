// $Header: /soft/cvsroot/AliRoot/EVE/macros/isegv.C,v 1.1.1.1 2006/05/09 11:38:49 hristov Exp $
// Cause ROOT to properly crash and dump core on SigSEGV.

{
 gSystem->IgnoreSignal(kSigSegmentationViolation, true);
}
