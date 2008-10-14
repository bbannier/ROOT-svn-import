// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

#include "Reflex/Reflex.h"
#include <iostream>
#include <fstream>

#ifdef _WIN32
  #include<windows.h>
#elif defined(__linux) || defined (__APPLE__)
  #include<dlfcn.h>
#endif

using namespace Reflex;
using namespace std;

enum Visibility { Public, Protected, Private }; 

void generate_visibility( ostream& out, const Member& m, const string& indent, Visibility& v ) {
  if ( m.Is(kPublic) && v != Public ) {
    out << indent << "public:" << endl;  v = Public;
  } else if ( m.Is(kProtected) && v != Protected ) {
    out << indent << "protected:" << endl;  v = Protected;
  } else if ( m.Is(kPrivate) && v != Private ) {
    out << indent << "private:" << endl;  v = Private;
  }
}
void generate_comment( ostream& out, const Member& m ) {
  if ( m.Properties().HasProperty("comment") ) {
    out << "  //" << m.Properties().PropertyAsString("comment");
  }
}

void generate_class( ostream& out, const Type& cl, const string& indent = "" ) {
  out << indent << "class " << cl.Name();
  //...Bases
  if (cl.Bases().Size() != 0 ) {
    out << " : " ;
    const Base_Iterator baseEnd = cl.Bases().End();
    for ( Base_Iterator base = cl.Bases().Begin(); base != baseEnd; ) {
      Base ba = *base;
      if ( ba.Is(kVirtual) ) out << "virtual ";
      if ( ba.Is(kPublic) ) out << "public ";
      if ( ba.Is(kPrivate) ) out << "private ";
      out << ba.ToType().Name(Reflex::kScoped);
      ++base;
      if ( base != baseEnd ) out << ", ";
    }
  }
  out << " {" << endl;
  Visibility curr_vis = Private;
  //...data members
  const Member_Iterator iDMEnd = ((Scope)cl).DataMembers().End();
  for ( Member_Iterator iDM = ((Scope)cl).DataMembers().Begin(); iDM != iDMEnd; ++iDM ) {
    Member dm = *iDM;
    if ( dm.Is(kArtificial) ) continue;
    generate_visibility(out, dm, indent, curr_vis);
    out << indent + "  " << dm.TypeOf().Name(Reflex::kScoped|Reflex::kQualified) << " " << dm.Name() <<";" ;
    generate_comment( out, dm );
    out << endl;
  }
  //...methods
  const Member_Iterator iFMEnd = ((Scope)cl).FunctionMembers().End();
  for ( Member_Iterator iFM = ((Scope)cl).FunctionMembers().Begin(); iFM != iFMEnd; ++iFM ) {
    Member fm = *iFM;
    if ( fm.Is(kArtificial) ) continue;
    generate_visibility( out, fm, indent, curr_vis);
    Type ft = fm.TypeOf();
    out << indent + "  ";
    if ( ! fm.Is(kConstructor) && !fm.Is(kDestructor) ) out << ft.ReturnType().Name(Reflex::kScoped) << " ";
    if (  fm.Is(kOperator) ) out << "operator ";
    out << fm.Name() << " (";
    if ( ft.FunctionParameters().Size() == 0 ) {
      out << "void";
    } else {
      const Type_Iterator iParEnd = ft.FunctionParameters().End();
      String_Iterator iParName = fm.FunctionParameterNames().Begin();
      String_Iterator iParDef = fm.FunctionParameterDefaults().Begin();
      for ( Type_Iterator iPar = ft.FunctionParameters().Begin() ; iPar != iParEnd;
            /* ++iPar below */ ++iParName, ++iParDef ) {
        out << iPar->Name(Reflex::kScoped|Reflex::kQualified);
        if ( *iParName != "" ) out << " " << *iParName;
        if ( *iParDef  != "" ) out << " = " << *iParDef;
        ++iPar;
        if ( iPar != iParEnd ) out << ", ";
      }
    }
    out << ");";
    generate_comment( out, fm );
    out << endl;
  }

  out << indent << "};" << endl;
}

template <typename T>
struct NameSorter {
   bool operator()(const T& one, const T& two) const {
      return one.Name(0) < two.Name(0);
   }
};

void generate_namespace(ostream& out, const Scope& ns, const string& indent = "" ) {

  if ( ! ns.IsTopScope() ) out << indent << "namespace "<< ns.Name() << " {" << endl;


  // Sub-Namespaces
  std::vector<Scope> subscopes(ns.SubScopes().Begin(), ns.SubScopes().End());
  std::sort(subscopes.begin(), subscopes.end(), NameSorter<Scope>());

  for ( size_t i = 0; i < subscopes.size(); i++ ) {
    Scope sc = subscopes[i];
    if ( sc.Is(kNamespace) ) generate_namespace(out, sc, indent + "  ");
    //one is enough, and we already generate classes as types below:
    // if ( sc.Is(kClassOrStruct) ) generate_class(out, Type::ByName(sc.Name(Reflex::kScoped)), indent + "  ");
  }
  // Types----
  std::vector<Type> subtypes(ns.SubTypes().Begin(), ns.SubTypes().End());
  std::sort(subtypes.begin(), subtypes.end(), NameSorter<Type>());

  for ( size_t t = 0; t < subtypes.size(); t++ ) {
    Type ty = subtypes[t];
    if ( ty.Is(kClassOrStruct) ) generate_class(out, ty, indent + "  ");
  }

  if ( ! ns.IsTopScope() ) out << indent << "}" << endl;
}

int main(int /*argc*/, char* argv[]) {

#ifdef _WIN32
  HMODULE libInstance = LoadLibrary("libtest_Class2DictRflx.dll");
  if ( ! libInstance )  std::cerr << "Could not load dictionary. " << std::endl << "Reason: " << GetLastError() << std::endl;
#else
  void * libInstance = dlopen("libtest_Class2DictRflx.so", RTLD_LAZY);
  if ( ! libInstance )  std::cerr << "Could not load dictionary. " << std::endl << "Reason: " << dlerror() << std::endl;
#endif

  std::string outfilename(argv[0]);
  outfilename += ".testout";
  ofstream outfile(outfilename.c_str());

  generate_namespace(outfile, Scope::GlobalScope());

  int ret = 0;
#if defined (_WIN32)
  ret = FreeLibrary(libInstance);
  if (ret == 0) std::cerr << "Unload of dictionary library failed." << std::endl << "Reason: " << GetLastError() << std::endl;
#else
  ret = dlclose(libInstance);
  if (ret == -1) std::cerr << "Unload of dictionary library failed." << std::endl << "Reason: " << dlerror() << std::endl;
#endif

  return 0;
}

 
