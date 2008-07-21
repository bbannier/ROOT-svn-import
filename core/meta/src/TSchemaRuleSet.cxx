// @(#)root/core:$Id$
// author: Lukasz Janyst <ljanyst@cern.ch>

#include "TSchemaRuleSet.h"
#include "TSchemaRule.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TClass.h"

#include <iostream>

ClassImp(TSchemaRule)

using namespace ROOT;

//------------------------------------------------------------------------------
TSchemaRuleSet::TSchemaRuleSet(): fPersistentRules( 0 ), fRemainingRules( 0 ),
                                  fAllRules( 0 ), fClass( 0 )
{
   fPersistentRules = new TObjArray();
   fRemainingRules  = new TObjArray();
   fAllRules        = new TObjArray();
   fAllRules->SetOwner( kFALSE );
}

//------------------------------------------------------------------------------
TSchemaRuleSet::~TSchemaRuleSet()
{
   delete fPersistentRules;
   delete fRemainingRules;
   delete fAllRules;
}

//------------------------------------------------------------------------------
Bool_t TSchemaRuleSet::AddRule( TSchemaRule* rule, Bool_t checkConsistency )
{
   //---------------------------------------------------------------------------
   // Cannot verify the consistency if the TClass object is not present
   //---------------------------------------------------------------------------
   if( checkConsistency && !fClass )
      return kFALSE;

   if( !rule->IsValid() )
      return kFALSE;

   //---------------------------------------------------------------------------
   // If we don't check the consistency then we should just add the object
   //---------------------------------------------------------------------------
   if( !checkConsistency ) {
      if( rule->GetEmbed() )
         fPersistentRules->Add( rule );
      else
         fRemainingRules->Add( rule );
      fAllRules->Add( rule );
   }

   //---------------------------------------------------------------------------
   // Check if all of the target data members specified in the rule are
   // present int the target class
   //---------------------------------------------------------------------------
   TObject*      obj;
   TObjArrayIter it( rule->GetTarget() );
   while( (obj = it.Next()) ) {
      TObjString* str = (TObjString*)obj;
      if( !fClass->GetDataMember( str->GetString() ) )
         return kFALSE;
   }

   //---------------------------------------------------------------------------
   // Check if there is a rule conflicting with this one
   //---------------------------------------------------------------------------

   return kTRUE;
}

//------------------------------------------------------------------------------
Bool_t TSchemaRuleSet::AddRules( TObjArray* rules, Bool_t checkConsistency )
{
   return kFALSE;
}

//------------------------------------------------------------------------------
const TObjArray* TSchemaRuleSet::FindRules( TString source ) const
{
   // User has to delete the returned array

   TObject*      obj;
   TObjArrayIter it( fAllRules );
   TObjArray*    arr = new TObjArray();
   arr->SetOwner( kFALSE );

   while( (obj = it.Next()) ) {
      TSchemaRule* rule = (TSchemaRule*)obj;
      if( rule->GetSourceClass() == source )
         arr->Add( rule );
   }
   return arr;
}

//------------------------------------------------------------------------------
const TObjArray* TSchemaRuleSet::FindRules( TString source, Int_t version ) const
{
   // User has to delete the returned array

   TObject*      obj;
   TObjArrayIter it( fAllRules );
   TObjArray*    arr = new TObjArray();
   arr->SetOwner( kFALSE );

   while( (obj = it.Next()) ) {
      TSchemaRule* rule = (TSchemaRule*)obj;
      if( rule->GetSourceClass() == source && rule->TestVersion( version ) )
         arr->Add( rule );
   }
   return arr;
}

//------------------------------------------------------------------------------
const TObjArray* TSchemaRuleSet::FindRules( TString source, UInt_t checksum ) const
{
   // User has to delete the returned array

   TObject*      obj;
   TObjArrayIter it( fAllRules );
   TObjArray*    arr = new TObjArray();
   arr->SetOwner( kFALSE );

   while( (obj = it.Next()) ) {
      TSchemaRule* rule = (TSchemaRule*)obj;
      if( rule->GetSourceClass() == source && rule->TestChecksum( checksum ) )
         arr->Add( rule );
   }
   return arr;
}

//------------------------------------------------------------------------------
TClass* TSchemaRuleSet::GetClass()
{
   return fClass;
}

//------------------------------------------------------------------------------
UInt_t TSchemaRuleSet::GetClassChecksum() const
{
   return fChecksum;
}

//------------------------------------------------------------------------------
TString TSchemaRuleSet::GetClassName() const
{
   return fClassName;
}

//------------------------------------------------------------------------------
Int_t TSchemaRuleSet::GetClassVersion() const
{
   return fVersion;
}

//------------------------------------------------------------------------------
const TObjArray* TSchemaRuleSet::GetRules() const
{
   return fAllRules;
}

//------------------------------------------------------------------------------
const TObjArray* TSchemaRuleSet::GetPersistentRules() const
{
   return fPersistentRules;
}

//------------------------------------------------------------------------------
void TSchemaRuleSet::RemoveRule( TSchemaRule* rule )
{
}

//------------------------------------------------------------------------------
void TSchemaRuleSet::RemoveRules( TObjArray* rules )
{
}

//------------------------------------------------------------------------------
void TSchemaRuleSet::SetClass( TClass* cls )
{
   fClass     = cls;
   fClassName = cls->GetName();
   fVersion   = cls->GetClassVersion();
   fChecksum  = cls->GetCheckSum();
}
