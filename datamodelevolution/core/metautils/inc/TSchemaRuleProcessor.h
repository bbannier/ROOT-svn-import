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
         static void SplitDeclaration( const std::string& source,
                                       std::list<std::pair<std::string,std::string> >& result)
         {
            // Split the string producing a list of substrings


            result.clear();

            //------------------------------------------------------------------
            // Split the list of sources
            //------------------------------------------------------------------
            std::list<std::string> sourceList;
            SplitList( source, sourceList );

            //------------------------------------------------------------------
            // Check if the type was declared
            //------------------------------------------------------------------
            std::list<std::string>::iterator it;
            std::string::size_type           space;

            for( it = sourceList.begin(); it != sourceList.end(); ++it ) {
               std::string elem = "";
               std::string type = "";

               if( (space = it->find( " " )) == std::string::npos )
                  elem = *it;
               else {
                  type = it->substr( 0, space );
                  elem = it->substr( space+1, it->size() );
               }
               result.push_back( std::make_pair( type, elem ) );
            }
         }

         //---------------------------------------------------------------------
         static std::string Trim( const std::string& source, char character = ' ' )
         {
            // Trim the whitespaces at the beginning and at the end of
            // given source string

            std::string::size_type start, end;
            for( start = 0; start < source.size() && isspace(source[start]); ++start) {}
            if( start == source.size() )
               return "";
            for( end = source.size()-1; end > start && source[end] == character; --end ) ;
            return source.substr( start, end-start+1 );
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

            std::string version = Trim( source );

            if( version.empty() )
               return false;

            //------------------------------------------------------------------
            // Check if we have a minus somewhere, if not then single version
            // number was specified
            //------------------------------------------------------------------
            hyphenI = version.find( '-' );
            if( hyphenI == std::string::npos && IsANumber( version ) ) {
               result.first = result.second = atoi( version.c_str() );
               return true;
            }

            //------------------------------------------------------------------
            // We start with the hyphen
            //------------------------------------------------------------------
            if( hyphenI == 0 ) {
               second = Trim( version.substr( 1 ) );
               if( IsANumber( second ) ) {
                  result.first  = -10;
                  result.second = atoi( second.c_str() );
                  return true;
               }
            }

            //------------------------------------------------------------------
            // We end with the hyphen
            //------------------------------------------------------------------
            if( hyphenI == version.size()-1 ) {
               first = Trim( version.substr( 0, version.size()-1 ) );
               if( IsANumber( first ) ) {
                  result.first  = atoi( first.c_str() );
                  result.second = 50000;
                  return true;
               }
            }

            //------------------------------------------------------------------
            // We have the hyphen somewhere in the middle
            //------------------------------------------------------------------
            first  = Trim( version.substr( 0, hyphenI ) );
            second = Trim( version.substr( hyphenI+1, version.size()-hyphenI-1 ) );
            if( IsANumber( first ) && IsANumber( second ) ) {
               result.first  = atoi( first.c_str() );
               result.second = atoi( second.c_str() );
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
