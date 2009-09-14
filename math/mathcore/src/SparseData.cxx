// @(#)root/mathcore:$Id: BinData.cxx 29267 2009-06-30 10:16:44Z moneta $
// Author: David Gonzalez Maline Wed Aug 28 15:33:03 2009

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Implementation file for class BinData

#include <iostream>
#include <iterator>
#include <algorithm>

#include <vector>
#include <list>

#include <cmath>

#include "Fit/SparseData.h"

#include "TH1.h"

using namespace std;

namespace ROOT { 

   namespace Fit { 

      class Box
      {
      public:
         Box(vector<double>& min, vector<double>& max, double value = 0.0, double error = 1.0):
            _min(min), _max(max), _val(value), _error(error)
         { }
         
         bool operator==(const Box& b)
         { return (_min == b._min) && (_max == b._max) 
               && (_val == b._val) && (_error == b._error);  }
         
         const vector<double>& getMin() const { return _min; }
         const vector<double>& getMax() const { return _max; }
         double getVal() const { return _val; }
         double getError() const { return _error; }
         
         void addVal(const double value) { _val += value; }
         
         friend class BoxContainer;
         friend ostream& operator <<(ostream& os, const Box& b);
         
      private:
         vector<double> _min;
         vector<double> _max;
         double _val;
         double _error;
      };
      
      class BoxContainer
      {
      private:
         const Box& _b;
      public:
         BoxContainer(const Box& b): _b(b) {}
         
         bool operator() (const Box& b1)
         { return operator()(_b, b1);  }
         
         // Looks if b2 is included in b1
         bool operator() (const Box& b1, const Box& b2)
         {
            bool isIn = true;
            vector<double>::const_iterator boxit = b2._min.begin();
            vector<double>::const_iterator bigit = b1._max.begin();
            while ( isIn && boxit != b2._min.end() )
            {
               if ( (*boxit) >= (*bigit) ) isIn = false;
               boxit++;
               bigit++;
            }
            
            boxit = b2._max.begin();
            bigit = b1._min.begin();
            while ( isIn && boxit != b2._max.end() )
            {
               if ( (*boxit) <= (*bigit) ) isIn = false;
               boxit++;
               bigit++;
            }
            
            return isIn;
         }
      };
      
      class AreaComparer
      {
      public:
         AreaComparer(vector<double>::iterator iter, double cmpLimit = 1e-16): 
            thereIsArea(true), 
            it(iter),
            limit(cmpLimit)
         {};
         
         void operator() (double value)
         {
            if ( fabs(value- (*it)) < limit )
               thereIsArea = false;
            
            it++;
         }
         
         bool isThereArea() { return thereIsArea; }
         
      private:
         bool thereIsArea;
         vector<double>::iterator it;
         double limit;
      };
      
      void DivideBox( const vector<double>& min, const vector<double>& max,
                      const vector<double>& bmin, const vector<double>& bmax,
                      const unsigned int size, const unsigned int n,
                      list<Box>& l, const double val, const double error)
      {
         vector<double> boxmin(min);
         vector<double> boxmax(max);
         
         boxmin[n] = min[n];
         boxmax[n] = bmin[n];
         if ( for_each(boxmin.begin(), boxmin.end(), AreaComparer(boxmax.begin())).isThereArea() )
            l.push_back(Box(boxmin, boxmax));
         
         boxmin[n] = bmin[n];
         boxmax[n] = bmax[n];
         if ( n == 0 ) 
         {
            if ( for_each(boxmin.begin(), boxmin.end(), AreaComparer(boxmax.begin())).isThereArea() )
               l.push_back(Box(boxmin, boxmax, val, error));
         }
         else
            DivideBox(boxmin, boxmax, bmin, bmax, size, n-1, l, val, error);
         
         boxmin[n] = bmax[n];
         boxmax[n] = max[n];
         if ( for_each(boxmin.begin(), boxmin.end(), AreaComparer(boxmax.begin())).isThereArea() )
            l.push_back(Box(boxmin, boxmax));
      }
      
      class ProxyListBox
      {
      public:
         void push_back(Box& box) { l.push_back(box); }
         list<Box>::iterator begin() { return l.begin(); }
         list<Box>::iterator end() { return l.end(); }
         void remove(Box& box) { l.remove(box); }
         list<Box>& getList() { return l; }
      private:
         list<Box> l;
      };



      SparseData::SparseData(TH1* h1)
      {
         int dim(h1->GetDimension());
         vector<double> min(dim);
         vector<double> max(dim);

         min[0] = h1->GetXaxis()->GetXmin();
         max[0] = h1->GetXaxis()->GetXmax();
         if ( dim <= 2 )
         {
            min[1] = h1->GetYaxis()->GetXmin();
            max[1] = h1->GetYaxis()->GetXmax();
         } 
         if ( dim <= 3 ) {
            min[2] = h1->GetZaxis()->GetXmin();
            max[2] = h1->GetZaxis()->GetXmax();
         }
         Box originalBox(min, max);
         l = new ProxyListBox();
         l->push_back(originalBox);
         
//          cout << "Original List: -" << endl;
//          PrintList();
//          cout << "END OF PRINTING" << endl;
//          cout << endl;
      }

      SparseData::~SparseData()
      { delete l; }


      void SparseData::Add(std::vector<double>& min, std::vector<double>& max, 
                           const double content, const double error)
      {
         Box littleBox(min, max);
         list<Box>::iterator it;
         it = std::find_if(l->begin(), l->end(), BoxContainer(littleBox));
         if ( it != l->end() )
//             cout << "Found: " << *it << endl;
            ;
         else
            cout << "FAILED!" << endl;
         if ( it->getVal() )
            it->addVal( content );
         else
         {
            DivideBox(it->getMin(), it->getMax(),
                      littleBox.getMin(), littleBox.getMax(),
                      it->getMin().size(), it->getMin().size() - 1,
                      l->getList(), content, error );
            l->remove(*it);
         }
      }

      void SparseData::PrintList()
      {
         copy(l->begin(), l->end(), ostream_iterator<Box>(cout, "\n------\n"));
      }


      BinData* SparseData::GetBinData()
      {
         list<Box>::iterator it = l->begin();
         const unsigned int dim = it->getMin().size();

         BinData * bd = new BinData(l->getList().size(), dim, BinData::kValueError);
         for ( ; it != l->end(); ++it )
         {
            vector<double> mid(dim);
            for ( unsigned int i = 0; i < dim; ++i)
            {
               mid[i] = ((it->getMax()[i] - it->getMin()[i]) /2) + it->getMin()[i];
            }
           
            bd->Add(&mid[0], it->getVal(), it->getError());
         }
         
         return bd;
      }

      BinData* SparseData::GetBinDataIntegral()
      {
         list<Box>::iterator it = l->begin();
         const unsigned int dim = it->getMin().size();

         ROOT::Fit::DataOptions opt; 
         opt.fIntegral=true;
         BinData * bd = new BinData(opt,l->getList().size(), dim, BinData::kValueError);
         for ( ; it != l->end(); ++it )
         {
            bd->Add(&(it->getMin()[0]), it->getVal(), it->getError());
            bd->AddBinUpEdge(&(it->getMax()[0]));
         }
         
         return bd;         
      }

      BinData* SparseData::GetBinDataNoCeros()
      {
         list<Box>::iterator it = l->begin();
         const unsigned int dim = it->getMin().size();

         BinData * bd = new BinData(l->getList().size(), dim, BinData::kValueError);
         for ( ; it != l->end(); ++it )
         {
            if ( it->getVal() == 0 ) continue;
            vector<double> mid(dim);
            for ( unsigned int i = 0; i < dim; ++i)
            {
               mid[i] = ((it->getMax()[i] - it->getMin()[i]) /2) + it->getMin()[i];
            }
           
            bd->Add(&mid[0], it->getVal(), it->getError());
         }
         
         return bd;
      }

      // Just for debugging pourposes
      ostream& operator <<(ostream& os, const ROOT::Fit::Box& b)
      {
         os << "min: ";
         copy(b.getMin().begin(), b.getMin().end(), ostream_iterator<double>(os, " "));
         os << "max: ";
         copy(b.getMax().begin(), b.getMax().end(), ostream_iterator<double>(os, " "));
         os << "val: " << b.getVal();
         
         return os;
      }     
   } // end namespace Fit

} // end namespace ROOT
