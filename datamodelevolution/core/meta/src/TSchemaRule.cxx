// @(#)root/core:$Id$
// author: Lukasz Janyst <ljanyst@cern.ch>

#include "TSchemaRule.h"
#include "TSchemaRuleProcessor.h"
#include "TObjArray.h"
#include "TObjString.h"
#include <utility>
#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <cstdlib>

ClassImp(TSchemaRule)

using namespace ROOT;

//------------------------------------------------------------------------------
TSchemaRule::TSchemaRule(): fVersionVect( 0 ), fChecksumVect( 0 ),
                            fTargetVect( 0 ), fSourceVect( 0 ),
                            fIncludeVect( 0 ), fEmbed( kFALSE ), 
                            fReadFuncPtr( 0 ), fReadRawFuncPtr( 0 ),
                            fRuleType( kNone )
{
}

//------------------------------------------------------------------------------
TSchemaRule::~TSchemaRule()
{
   delete fVersionVect;
   delete fChecksumVect;
   delete fTargetVect;
   delete fSourceVect;
   delete fIncludeVect;
}

//------------------------------------------------------------------------------
TSchemaRule::TSchemaRule( const TSchemaRule& rhs ): TObject( rhs )
{
   *this = rhs;
}

//------------------------------------------------------------------------------
TSchemaRule& TSchemaRule::operator = ( const TSchemaRule& rhs )
{
   if( this != &rhs ) {
      fVersion        = rhs.fVersion;
      fChecksum       = rhs.fChecksum;
      fSourceClass    = rhs.fSourceClass;
      fTarget         = rhs.fTarget;
      fSource         = rhs.fSource;
      fInclude        = rhs.fInclude;
      fCode           = rhs.fCode;
      fEmbed          = rhs.fEmbed;
      fReadFuncPtr    = rhs.fReadFuncPtr;
      fReadRawFuncPtr = rhs.fReadRawFuncPtr;
      fRuleType       = rhs.fRuleType;
   }
   return *this;
}

//------------------------------------------------------------------------------
Bool_t TSchemaRule::SetVersion( const TString& version )
{
   // Set the version string - returns kFALSE if the format is incorrect

   fVersion = "";
   Bool_t ret = ProcessVersion( version );
   if( ret )
      fVersion = version;
   return ret;
}

//------------------------------------------------------------------------------
Bool_t TSchemaRule::TestVersion( Int_t version ) const
{
   // Check if given version number is defined in this rule

   if( fVersion == "" )
      return kFALSE;

   if( !fVersionVect )
      ProcessVersion( fVersion ); // At this point the version string should always be correct

   std::vector<std::pair<Int_t, Int_t> >::iterator it;
   for( it = fVersionVect->begin(); it != fVersionVect->end(); ++it ) {
      if( version >= it->first && version <= it->second )
         return kTRUE;
   }
   return kFALSE;
}

//------------------------------------------------------------------------------
Bool_t TSchemaRule::SetChecksum( const TString& checksum )
{
   // Set the checksum string - returns kFALSE if the format is incorrect
   fChecksum = "";
   Bool_t ret = ProcessChecksum( checksum );
   if( ret )
      fVersion = "";
   return ret;
}

//------------------------------------------------------------------------------
Bool_t TSchemaRule::TestChecksum( UInt_t checksum ) const
{
   // Check if given checksum is defined in this rule

   if( fChecksum == "" )
      return kFALSE;

   if( !fChecksumVect )
      ProcessChecksum( fChecksum ); // At this point the checksum string should always be correct

   std::vector<UInt_t>::iterator it;
   for( it = fChecksumVect->begin(); it != fChecksumVect->end(); ++it ) {
      if( checksum == *it )
         return kTRUE;
   }
   return kFALSE;
}

//------------------------------------------------------------------------------
void TSchemaRule::SetSourceClass( const TString& classname )
{
   fSourceClass = classname;
}

//------------------------------------------------------------------------------
TString TSchemaRule::GetSourceClass() const
{
   return fSourceClass;
}

//------------------------------------------------------------------------------
void TSchemaRule::SetTarget( const TString& target )
{
   fTarget = target;

   if( target == "" ) {
      delete fTargetVect;
      fTargetVect = 0;
      return;
   }

   if( !fTargetVect )
      fTargetVect = new TObjArray();

   ProcessList( fTargetVect, target );
}

//------------------------------------------------------------------------------
const TObjArray*  TSchemaRule::GetTarget() const
{
   if( fTarget == "" )
      return 0;

   if( !fTargetVect ) {
      fTargetVect = new TObjArray();
      ProcessList( fTargetVect, fTarget );
   }

   return fTargetVect;
}

//------------------------------------------------------------------------------
void TSchemaRule::SetSource( const TString& source )
{
   fSource = source;

   if( source == "" ) {
      delete fSourceVect;
      fSourceVect = 0;
      return;
   }

   if( !fSourceVect )
      fSourceVect = new TObjArray();

   ProcessList( fSourceVect, source );
}

//------------------------------------------------------------------------------
const TObjArray* TSchemaRule::GetSource() const
{
   if( fSource == "" )
      return 0;

   if( !fSourceVect ) {
      fSourceVect = new TObjArray();
      ProcessList( fSourceVect, fSource );
   }
   return fSourceVect;
}

//------------------------------------------------------------------------------
void TSchemaRule::SetInclude( const TString& incl )
{
   fInclude = incl;

   if( incl == "" ) {
      delete fIncludeVect;
      fIncludeVect = 0;
      return;
   }

   if( !fIncludeVect )
      fIncludeVect = new TObjArray();

   ProcessList( fIncludeVect, incl );
}

//------------------------------------------------------------------------------
const TObjArray* TSchemaRule::GetInclude() const
{
   if( fInclude == "" )
      return 0;

   if( !fIncludeVect ) {
      fIncludeVect = new TObjArray();
      ProcessList( fIncludeVect, fInclude );
   }

   return fIncludeVect;
}

//------------------------------------------------------------------------------
void TSchemaRule::SetEmbed( Bool_t embed )
{
   fEmbed = embed;
}

//------------------------------------------------------------------------------
Bool_t TSchemaRule::GetEmbed() const
{
   return fEmbed;
}

//------------------------------------------------------------------------------
Bool_t TSchemaRule::IsValid() const
{
   return (fVersionVect || fChecksumVect) && (fSourceClass.Length() != 0)
          && fTargetVect; 
   // It is okay to not have a source (transient member setting) 
}

//------------------------------------------------------------------------------
void TSchemaRule::SetCode( const TString& code )
{
   fCode = code;
}

//------------------------------------------------------------------------------
TString TSchemaRule::GetCode() const
{
   return fCode;
}

//------------------------------------------------------------------------------
Bool_t TSchemaRule::HasTarget( const TString& target ) const
{
   if( !fTarget )
      return kFALSE;

   TObject*      obj;
   TObjArrayIter it( fTargetVect );
   while( (obj = it.Next()) ) {
      TObjString* str = (TObjString*)obj;
      if( str->GetString() == target )
         return kTRUE;
   }
   return kFALSE;
}

//------------------------------------------------------------------------------
Bool_t TSchemaRule::HasSource( const TString& source ) const
{
   if( !fSource )
      return kFALSE;

   TObject*      obj;
   TObjArrayIter it( fSourceVect );
   while( (obj = it.Next()) ) {
      TObjString* str = (TObjString*)obj;
      if( str->GetString() == source )
         return kTRUE;
   }
   return kFALSE;
}

//------------------------------------------------------------------------------
void TSchemaRule::SetReadFunctionPointer( TSchemaRule::ReadFuncPtr_t ptr )
{
   fReadFuncPtr = ptr;
}

//------------------------------------------------------------------------------
TSchemaRule::ReadFuncPtr_t TSchemaRule::GetReadFunctionPointer() const
{
   return fReadFuncPtr;
}

//------------------------------------------------------------------------------
void TSchemaRule::SetReadRawFunctionPointer( TSchemaRule::ReadRawFuncPtr_t ptr )
{
   fReadRawFuncPtr = ptr;
}

//------------------------------------------------------------------------------
TSchemaRule::ReadRawFuncPtr_t TSchemaRule::GetReadRawFunctionPointer() const
{
   return fReadRawFuncPtr;
}

//------------------------------------------------------------------------------
void TSchemaRule::SetRuleType( TSchemaRule::RuleType_t type )
{
   fRuleType = type;
}

//------------------------------------------------------------------------------
TSchemaRule::RuleType_t TSchemaRule::GetRuleType() const
{
   return fRuleType;
}

//------------------------------------------------------------------------------
Bool_t TSchemaRule::Conflicts( const TSchemaRule* rule ) const
{
   // Check if this rule conflicts with the given one

   //---------------------------------------------------------------------------
   // If the rules have different sources then the don't conflict
   //---------------------------------------------------------------------------
   if( GetSourceClass() != rule->GetSourceClass() )
      return kFALSE;

   //---------------------------------------------------------------------------
   // Check if the rules have common target
   //---------------------------------------------------------------------------
   if( !rule->GetTarget() )
      return kFALSE;

   Bool_t         haveCommonTargets = kFALSE;
   TObjArrayIter  titer( rule->GetTarget() );
   TObjString    *str;
   TObject       *obj;

   while( (obj = titer.Next() ) ) {
      str = (TObjString*)obj;
      if( HasTarget( str->String() ) )
         haveCommonTargets = kTRUE;
   }

   if( !haveCommonTargets )
      return kFALSE;

   //---------------------------------------------------------------------------
   // Check if there are conflicting checksums
   //---------------------------------------------------------------------------
   if( fChecksumVect ) {
      std::vector<UInt_t>::iterator it;
      for( it = fChecksumVect->begin(); it != fChecksumVect->end(); ++it )
         if( rule->TestChecksum( *it ) )
            return kTRUE;
   }

   //---------------------------------------------------------------------------
   // Check if there are conflicting versions
   //---------------------------------------------------------------------------
   if( fVersionVect && rule->fVersionVect )
   {
      std::vector<std::pair<Int_t, Int_t> >::iterator it1;
      std::vector<std::pair<Int_t, Int_t> >::iterator it2;
      for( it1 = fVersionVect->begin(); it1 != fVersionVect->end(); ++it1 ) {
         for( it2 = rule->fVersionVect->begin();
              it2 != rule->fVersionVect->end(); ++it2 ) {
            //------------------------------------------------------------------
            // the rules conflict it their version ranges intersect
            //------------------------------------------------------------------
            if( it1->first >= it2->first && it1->first <= it2->second )
               return kTRUE;

            if( it1->first < it2->first && it1->second >= it2->first )
               return kTRUE;
         }
      }
   }
   return kFALSE;
}

//------------------------------------------------------------------------------
Bool_t TSchemaRule::ProcessVersion( const TString& version ) const
{
   // Check if specified version string is correct and build version vector

   //---------------------------------------------------------------------------
   // Check if we have valid list
   //---------------------------------------------------------------------------
   std::string ver = (const char*)version;
   if( ver[0] != '[' || ver[ver.size()-1] != ']' )
      return kFALSE;

   std::list<std::string> versions;
   ROOT::TSchemaRuleProcessor::SplitList( ver.substr( 1, ver.size()-2), versions );

   if( versions.empty() )
   {
      delete fVersionVect;
      fVersionVect = 0;
      return kFALSE;
   }

   if( !fVersionVect )
      fVersionVect = new std::vector<std::pair<Int_t, Int_t> >;
   fVersionVect->clear();

   //---------------------------------------------------------------------------
   // Check the validity of each list element
   //---------------------------------------------------------------------------
   std::list<std::string>::iterator it;
   for( it = versions.begin(); it != versions.end(); ++it ) {
      std::pair<Int_t, Int_t> ver;
      if( !ROOT::TSchemaRuleProcessor::ProcessVersion( *it, ver ) )
      {
         delete fVersionVect;
         fVersionVect = 0;
         return kFALSE;
      }
      fVersionVect->push_back( ver );
   }
   return kTRUE;
}

//------------------------------------------------------------------------------
Bool_t TSchemaRule::ProcessChecksum( const TString& checksum ) const
{
   //---------------------------------------------------------------------------
   // Check if we have valid list
   //---------------------------------------------------------------------------
   std::string chk = (const char*)checksum;
   if( chk[0] != '[' || chk[chk.size()-1] != ']' )
      return kFALSE;

   std::list<std::string> checksums;
   ROOT::TSchemaRuleProcessor::SplitList( chk.substr( 1, chk.size()-2), checksums );

   if( checksums.empty() ) {
      delete fChecksumVect;
      fChecksumVect = 0;
      return kFALSE; 
   }

   if( !fChecksumVect )
      fChecksumVect = new std::vector<UInt_t>;
   fChecksumVect->clear();

   //---------------------------------------------------------------------------
   // Check the validity of each list element
   //---------------------------------------------------------------------------
   std::list<std::string>::iterator it;
   for( it = checksums.begin(); it != checksums.end(); ++it ) {
      if( !ROOT::TSchemaRuleProcessor::IsANumber( *it ) ) {
         delete fChecksumVect;
         fChecksumVect = 0;
         return kFALSE;
      }
      fChecksumVect->push_back( atoi( it->c_str() ) );
   }
   return kTRUE;
}

//------------------------------------------------------------------------------
void TSchemaRule::ProcessList( TObjArray* array, const TString& list ) const
{
   // process given list of strings

   std::list<std::string>           elems;
   std::list<std::string>::iterator it;
   ROOT::TSchemaRuleProcessor::SplitList( (const char*)list, elems );

   array->Clear();

   if( elems.empty() )
      return;

   for( it = elems.begin(); it != elems.end(); ++it ) {
      TObjString *str = new TObjString;
      *str = it->c_str();
      array->Add( str );
   }
}
