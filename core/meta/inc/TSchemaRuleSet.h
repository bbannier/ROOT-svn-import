// @(#)root/core:$Id$
// author: Lukasz Janyst <ljanyst@cern.ch>

#ifndef ROOT_TSchemaRuleSet
#define ROOT_TSchemaRuleSet

class TObjArray;
class TClass;

#include "TObject.h"
#include "Rtypes.h"
#include "TString.h"

namespace ROOT {

   class TSchemaRule;

   class TSchemaRuleSet: public TObject
   {
      public:

         TSchemaRuleSet();
         virtual ~TSchemaRuleSet();

         Bool_t           AddRule( TSchemaRule* rule, Bool_t checkConsistency = kTRUE );
         const TObjArray* FindRules( const TString &source ) const;
         const TObjArray* FindRules( const TString &source, Int_t version ) const;
         const TObjArray* FindRules( const TString &source, UInt_t checksum ) const;
         TClass*          GetClass();
         UInt_t           GetClassChecksum() const;
         TString          GetClassName() const;
         Int_t            GetClassVersion() const;
         const TObjArray* GetRules() const;
         const TObjArray* GetPersistentRules() const;
         void             RemoveRule( TSchemaRule* rule );
         void             RemoveRules( TObjArray* rules );
         void             SetClass( TClass* cls );

         ClassDef( TSchemaRuleSet, 1 )

      private:
         TObjArray*                             fPersistentRules; //  Array of the rules that will be embeded in the file
         TObjArray*                             fRemainingRules;  //! Array of non-persisten rules - just for cleanup purposes - owns the elements
         TObjArray*                             fAllRules;        //! Array of all rules
         TClass*                                fClass;           //! Target class pointer (for consistency checking)
         TString                                fClassName;       //  Target class name
         Int_t                                  fVersion;         //  Target class version
         UInt_t                                 fChecksum;        //  Target class checksum
   };

} // End of Namespace ROOT 

#endif // ROOT_TSchemaRuleSet
