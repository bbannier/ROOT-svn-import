// Base selection class from which all 
// selection classes should be derived
#ifndef BASESELECTIONRULE_H
#define BASESELECTIONRULE_H

#include <string>
#include <map>
#include <list>

typedef std::map<std::string, std::string> attributesList; // The liste of selection rule's attributes (, name, pattern, ...)

enum ESelect{ // a rule could be selected, vetoed or we don't care about it
   kYes,
   kNo,
   kDontCare
};

typedef enum ESelect ESelect;


class BaseSelectionRule{
private:
   attributesList attributes; // list of the attributes of the selection/exclusion rule
   ESelect fIsSelected;       // selected/vetoed/don't care
   std::list<std::string> sub_patterns; // a list of subpatterns, generated form a pattern/proto_pattern attribute 
   std::list<std::string> file_sub_patterns; // a list of subpatterns, generated form a file_pattern attribute

   bool fMatchFound;          // this is true if this selection rule has been used at least once
   
public:
   BaseSelectionRule():fIsSelected(kNo),fMatchFound(false){}
   BaseSelectionRule(ESelect sel, const std::string& attributeName, const std::string& attributeValue);
   
   void setSelected(ESelect sel);
   ESelect getSelected();
   bool hasAttributeWithName(const std::string& attributeName); // returns true if there is an attribute with the specified name
   bool getAttributeValue(const std::string& attributeName, std::string& returnValue); // returns the value of the attribute with name attributeName
   void setAttributeValue(const std::string& attributeName, const std::string& attributeValue); // sets an attribute with name attribute name and value attributeValue
   
   const attributesList& getAttributes(); // returns the list of attributes
   void printAttributes(int level);       // prints the list of attributes - level is the number of tabs from the beginning of the line

   bool isSelected (const std::string& name, const std::string& prototype, const std::string& file_name, bool& dontCare, bool& noName, bool& file, bool isLinkdef); // for more detailed description look at the .cxx file

   void setMatchFound(bool match); // set fMatchFound
   bool getMatchFound();           // get fMatchFound

protected:
   void processPattern(const std::string& pattern, std::list<std::string>& out); // divides a pattern into a list of sub-patterns
   bool BeginsWithStar(const std::string& pattern); // returns true if a pattern begins with a star
   bool EndsWithStar(const std::string& pattern);   // returns true of a pattern ends with a star
   bool CheckPattern(const std::string& test, const std::string& pattern, std::list<std::string>& patterns_list, bool isLinkdef);
}; // cheks if the test string matches against the pattern (which has produced the list of sub-patterns patterns_list). There is 
// difference if we are processing linkdef.h or selection.xmlpatterns

#endif
