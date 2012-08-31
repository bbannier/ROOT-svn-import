// functions from Giovanni

#include "RooStats/RooSimultaneousOpt.h"
#include "RooProdPdf.h"
#include "RooProduct.h"
#include "RooSimultaneous.h"

#include "RooStats/ModelConfig.h"
#include "RooStats/RooStatsUtils.h"
#include <typeinfo>
#include <stdexcept>

using namespace std;

namespace RooStats { 


RooAbsPdf *Utils::FactorizePdf(const RooArgSet &observables, RooAbsPdf &pdf, RooArgList &constraints) {
    assert(&pdf);
    const std::type_info & id = typeid(pdf);
    if (id == typeid(RooProdPdf)) {
        //std::cout << " pdf is product pdf " << pdf.GetName() << std::endl;
        RooProdPdf *prod = dynamic_cast<RooProdPdf *>(&pdf);
        RooArgList newFactors; RooArgSet newOwned;
        RooArgList list(prod->pdfList());
        bool needNew = false;
        for (int i = 0, n = list.getSize(); i < n; ++i) {
            RooAbsPdf *pdfi = (RooAbsPdf *) list.at(i);
            RooAbsPdf *newpdf = FactorizePdf(observables, *pdfi, constraints);
            //std::cout << "    for " << pdfi->GetName() << "   newpdf  " << (newpdf == 0 ? "null" : (newpdf == pdfi ? "old" : "new"))  << std::endl;
            if (newpdf == 0) { needNew = true; continue; }
            if (newpdf != pdfi) { needNew = true; newOwned.add(*newpdf); }
            newFactors.add(*newpdf);
        }
        if (!needNew) { copyAttributes(pdf, *prod); return prod; }
        else if (newFactors.getSize() == 0) return 0;
        else if (newFactors.getSize() == 1) {
            RooAbsPdf *ret = (RooAbsPdf *) newFactors.first()->Clone(TString::Format("%s_obsOnly", pdf.GetName()));
            copyAttributes(pdf, *ret);
            return ret;
        }
        RooProdPdf *ret = new RooProdPdf(TString::Format("%s_obsOnly", pdf.GetName()), "", newFactors);
        ret->addOwnedComponents(newOwned);
        copyAttributes(pdf, *ret);
        return ret;
    } else if (id == typeid(RooSimultaneous) || id == typeid(RooSimultaneousOpt)) {
        RooSimultaneous *sim  = dynamic_cast<RooSimultaneous *>(&pdf);
        RooAbsCategoryLValue *cat = (RooAbsCategoryLValue *) sim->indexCat().Clone();
        int nbins = cat->numBins((const char *)0);
        TObjArray factorizedPdfs(nbins); RooArgSet newOwned;
        bool needNew = false;
        for (int ic = 0, nc = nbins; ic < nc; ++ic) {
            cat->setBin(ic);
            RooAbsPdf *pdfi = sim->getPdf(cat->getLabel());
            RooAbsPdf *newpdf = FactorizePdf(observables, *pdfi, constraints);
            factorizedPdfs[ic] = newpdf;
            if (newpdf == 0) { throw std::runtime_error(std::string("ERROR: channel ") + cat->getLabel() + " factorized to zero."); }
            if (newpdf != pdfi) { needNew = true; newOwned.add(*newpdf); }
        }
        RooSimultaneous *ret = sim;
        if (needNew) {
            ret = new RooSimultaneous(TString::Format("%s_obsOnly", pdf.GetName()), "", (RooAbsCategoryLValue&) sim->indexCat());
            for (int ic = 0, nc = nbins; ic < nc; ++ic) {
                cat->setBin(ic);
                RooAbsPdf *newpdf = (RooAbsPdf *) factorizedPdfs[ic];
                if (newpdf) ret->addPdf(*newpdf, cat->getLabel());
            }
            ret->addOwnedComponents(newOwned);
        }
        delete cat;
        if (id == typeid(RooSimultaneousOpt)) {
            RooSimultaneousOpt *newret = new RooSimultaneousOpt(*ret);
            newret->addOwnedComponents(RooArgSet(*ret));
            ret = newret;
        }
        copyAttributes(pdf, *ret);
        return ret;
    } else if (pdf.dependsOn(observables)) {
        return &pdf;
    } else {
        if (!constraints.contains(pdf)) constraints.add(pdf);
        return 0;
    }



}


   void Utils::FactorizeFunc(const RooArgSet &observables, RooAbsReal &func, RooArgList &obsTerms, RooArgList &constraints, bool /*debug */) {
    RooAbsPdf *pdf = dynamic_cast<RooAbsPdf *>(&func);
    if (pdf != 0) { 
       RooStats::FactorizePdf(observables, *pdf, obsTerms, constraints); 
       return; 
    }
    const std::type_info & id = typeid(func);
    if (id == typeid(RooProduct)) {
        RooProduct *prod = dynamic_cast<RooProduct *>(&func);
        RooArgSet components(prod->components());
        //std::cout << "Function " << func.GetName() << " is a RooProduct with " << components.getSize() << " components." << std::endl;
        std::auto_ptr<TIterator> iter(components.createIterator());
        for (RooAbsReal *funci = (RooAbsReal *) iter->Next(); funci != 0; funci = (RooAbsReal *) iter->Next()) {
            //std::cout << "  component " << funci->GetName() << " of type " << funci->ClassName() << std::endl;
            FactorizeFunc(observables, *funci, obsTerms, constraints);
        }
    } else if (func.dependsOn(observables)) {
        if (!obsTerms.contains(func)) obsTerms.add(func);
    } else {
        if (!constraints.contains(func)) constraints.add(func);
    }
}


RooAbsPdf *Utils::FullClonePdf(const RooAbsPdf *pdf, RooArgSet &holder, bool cloneLeafNodes) {
  // Clone all FUNC compents by copying all branch nodes
  RooArgSet tmp("RealBranchNodeList") ;
  pdf->branchNodeServerList(&tmp);
  tmp.snapshot(holder, cloneLeafNodes); 
  // Find the top level FUNC in the snapshot list
  return (RooAbsPdf*) holder.find(pdf->GetName());
}
RooAbsReal *Utils::FullCloneFunc(const RooAbsReal *pdf, RooArgSet &holder, bool cloneLeafNodes) {
  // Clone all FUNC compents by copying all branch nodes
  RooArgSet tmp("RealBranchNodeList") ;
  pdf->branchNodeServerList(&tmp);
  tmp.snapshot(holder, cloneLeafNodes); 
  // Find the top level FUNC in the snapshot list
  return (RooAbsReal*) holder.find(pdf->GetName());
}

void Utils::copyAttributes(const RooAbsArg &from, RooAbsArg &to) {
    if (&from == &to) return;
    const std::set<std::string> attribs = from.attributes();
    if (!attribs.empty()) {
        for (std::set<std::string>::const_iterator it = attribs.begin(), ed = attribs.end(); it != ed; ++it) to.setAttribute(it->c_str());
    }
    const std::map<std::string, std::string> strattribs = from.stringAttributes();
    if (!strattribs.empty()) {
        for (std::map<std::string,std::string>::const_iterator it = strattribs.begin(), ed = strattribs.end(); it != ed; ++it) to.setStringAttribute(it->first.c_str(), it->second.c_str());
    }
}




}  // end namespace RooStats
