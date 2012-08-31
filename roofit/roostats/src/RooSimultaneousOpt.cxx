#include "RooStats/RooSimultaneousOpt.h"
#include "RooStats/CachingNLL.h"
#include <RooCmdConfig.h>

namespace RooStats { 

RooAbsReal* 
RooSimultaneousOpt::createNLL(RooAbsData& data, const RooLinkedList& cmdList) 
{
    RooCmdConfig pc(Form("RooSimultaneousOpt::createNLL(%s)",GetName())) ;
    pc.defineSet("cPars","Constrain",0,0);
    RooArgSet *cPars = pc.getSet("cPars");
    return new cacheutils::CachingSimNLL(this, &data, cPars);
}

}
