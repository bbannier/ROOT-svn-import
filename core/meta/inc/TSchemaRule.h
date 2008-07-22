// @(#)root/core:$Id$
// author: Lukasz Janyst <ljanyst@cern.ch>

#ifndef ROOT_TSchemaRule
#define ROOT_TSchemaRule

class TBuffer;
class TVirtualObject;
class TObjArray;

#include "TObject.h"
#include "Rtypes.h"
#include "TString.h"

#include <vector>
#include <utility>

namespace ROOT {

   class TSchemaRule: public TObject
   {
      public:

         typedef enum
         {
            kReadRule    = 0,
            kReadRawRule = 1
         }  RuleType_t;

         typedef void (*ReadFuncPtr_t)( char*, TVirtualObject* );
         typedef void (*ReadRawFuncPtr_t)(char*, TBuffer*);

         TSchemaRule();
         virtual ~TSchemaRule();
         Bool_t           SetVersion( TString version );
         Bool_t           TestVersion( Int_t version ) const;
         Bool_t           SetChecksum( TString checksum );
         Bool_t           TestChecksum( UInt_t checksum ) const;
         void             SetSourceClass( TString classname );
         TString          GetSourceClass() const;
         void             SetTarget( TString target );
         const TObjArray* GetTarget() const;
         void             SetSource( TString source );
         const TObjArray* GetSource() const;
         void             SetEmbed( Bool_t embed );
         Bool_t           GetEmbed() const;
         Bool_t           IsValid() const;
         void             SetCode( TString code );
         TString          GetCode() const;
         Bool_t           HasTarget( TString target ) const;
         Bool_t           HasSource( TString source ) const;
         void             SetReadFunctionPointer( ReadFuncPtr_t ptr );
         ReadFuncPtr_t    GetReadFunctionPointer() const;
         void             SetReadRawFunctionPointer( ReadRawFuncPtr_t ptr );
         ReadRawFuncPtr_t GetReadRawFunctionPointer() const;
         void             SetRuleType( RuleType_t type );
         RuleType_t       GetRuleType() const;
         Bool_t           Conflicts( const TSchemaRule* rule ) const;

         ClassDef( TSchemaRule, 1 );

      private:
         TString                                fVersion;        //  Source version string
         std::vector<std::pair<Int_t, Int_t> >* fVersionVect;    //! Source version vector (for searching purposes)
         TString                                fChecksum;       //  Source checksum string
         std::vector<UInt_t>*                   fChecksumVect;   //! Source checksum vector (for searching purposes)
         TString                                fSourceClass;    //  Source clss
         TString                                fTarget;         //  Target data mamber string
         TObjArray*                             fTargetVect;     //! Target data member vector (for searching purposes)
         TString                                fSource;         //  Source data member string
         TObjArray*                             fSourceVect;     //! Source data member vector (for searching purposes)
         TString                                fCode;           //  User specified code snippet
         bool                                   fEmbed;          //! Value determining if the rule should be embedded
         ReadFuncPtr_t                          fReadFuncPtr;    //! Conversion function pointer for read rule
         ReadRawFuncPtr_t                       fReadRawFuncPtr; //! Conversion function pointer for readraw rule
         RuleType_t                             fRuleType;       //  Type of the rule
   };
} // End of namespace ROOT

#endif // ROOT_TSchemaRule
