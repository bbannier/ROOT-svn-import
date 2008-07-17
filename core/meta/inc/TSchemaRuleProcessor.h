// @(#)root/core:$Id$
// author: Lukasz Janyst <ljanyst@cern.ch>

#ifndef ROOT_TSchemaRuleProcessor
#define ROOT_TSchemaRuleProcessor

#include <string>
#include <list>
#include <utility>
#include <cstdlib>
#include <iostream>
#include "Rtypes.h"

namespace ROOT
{
   class TSchemaRuleProcessor
   {
      public:
         //---------------------------------------------------------------------
         static void SplitList( const std::string& source,
                                std::list<std::string>& result,
                                char delimiter=',')
         {
            // Split the string producing a list of substrings

            std::string::size_type curr;
            std::string::size_type last = 0;
            std::string::size_type size;
            std::string            elem;

            result.clear();

            while( last != source.size() ) {
               curr = source.find( delimiter, last );

               if( curr == std::string::npos ) {
                  curr = source.size()-1;
                  size = curr-last+1;
               }
               else size = curr-last;

               elem = Trim( source.substr( last, size ) );
               if( !elem.empty() )
                  result.push_back( elem );

               last = curr+1;
            }
         }

         //---------------------------------------------------------------------
         static std::string Trim( const std::string& source, char character = ' ' )
         {
            // Trim the whitespaces at the beginning and at the end of
            // given source string

            std::string::size_type f, l;
            f = source.find_first_not_of( character );
            if( f == std::string::npos )
               return "";
            for( l = source.size()-1; l > f && source[l] == character; --l ) ;
            return source.substr( f, l-f+1 );
         }

         //---------------------------------------------------------------------
         static bool ProcessVersion( const std::string& source,
                                     std::pair<Int_t, Int_t>& result )
         {
            // Check if a version is specified correctly
            // The result is set the following way:
            //   x  :  first = x   second = x
            //  -x  :  first = -10 second = x
            // x-y  :  first = x   second = y
            // x-   :  first = x   second = 50000
            // if the given string is invalid (false is returned)
            // then the state of the result is undefined

            std::string::size_type hyphenI;
            std::string            first;
            std::string            second;

            if( source.empty() )
               return false;

            //------------------------------------------------------------------
            // Check if we have a minus somewhere, if not then single version
            // number was specified
            //------------------------------------------------------------------
            hyphenI = source.find( '-' );
            if( hyphenI == std::string::npos && IsANumber( source ) ) {
               result.first = result.second = atoi( source.c_str() );
               return true;
            }

            //------------------------------------------------------------------
            // We start with the hyphen
            //------------------------------------------------------------------
            if( hyphenI == 0 ) {
               second = Trim( source.substr( 1 ) );
               if( IsANumber( second ) ) {
                  result.first  = -10;
                  result.second = atoi( second.c_str() );
                  return true;
               }
            }

            //------------------------------------------------------------------
            // We end with the hyphen
            //------------------------------------------------------------------
            if( hyphenI == source.size()-1 ) {
               first = Trim( source.substr( 0, source.size()-1 ) );
               if( IsANumber( first ) ) {
                  result.first  = atoi( first.c_str() );
                  result.second = 50000;
                  return true;
               }
            }

            //------------------------------------------------------------------
            // We have the hyphen somewhere in the middle
            //------------------------------------------------------------------
            first  = Trim( source.substr( 0, hyphenI ) );
            second = Trim( source.substr( hyphenI+1, source.size()-hyphenI-1 ) );
            if( IsANumber( first ) && IsANumber( second ) ) {
               result.first  = atoi( first.c_str() );
               result.second = atoi( first.c_str() );
               return true;
            }

            return false;
         }

         //---------------------------------------------------------------------
         static bool IsANumber( const std::string& source )
         {
            // check if given string si consisted of digits

            if( source.empty() )
               return false;

            std::string::size_type i;
            for( i = 0; i < source.size(); ++i )
               if( !isdigit( source[i] ) )
                  return false;
            return true;
         }
   };
}

#endif // ROOT_TSchemaRuleProcessor
