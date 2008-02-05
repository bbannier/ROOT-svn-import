//
// File generated by utils/src/rootcint_tmp at Tue Feb 13 17:31:08 2007

// Do NOT change. Changes will be lost next time file is generated
//

#include "RConfig.h"
#if !defined(R__ACCESS_IN_SYMBOL)
//Break the privacy of classes -- Disabled for the moment
#define private public
#define protected public
#endif

// Since CINT ignores the std namespace, we need to do so in this file.
namespace std {} using namespace std;
#include "ManualBase4.h"

#include "TClass.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"

// START OF SHADOWS

namespace ROOT {
   namespace Shadow {
   } // of namespace Shadow
} // of namespace ROOT
// END OF SHADOWS

/********************************************************
* base/src/ManualBase4.cxx
* CAUTION: DON'T CHANGE THIS FILE. THIS FILE IS AUTOMATICALLY GENERATED
*          FROM HEADER FILES LISTED IN G__setup_cpp_environmentXXX().
*          CHANGE THOSE HEADER FILES AND REGENERATE THIS FILE.
********************************************************/

#ifdef G__MEMTEST
#undef malloc
#undef free
#endif

extern "C" void G__cpp_reset_tagtableManualBase4();

extern "C" void G__set_cpp_environmentManualBase4() {
  G__add_compiledheader("base/inc/TObject.h");
  G__add_compiledheader("base/inc/TMemberInspector.h");
  G__add_compiledheader("TDirectory.h");
  G__cpp_reset_tagtableManualBase4();
}
#include <new>
extern "C" int G__cpp_dllrevManualBase4() { return(30051515); }

/*********************************************************
* Member function Interface Method
*********************************************************/

/* TDirectory */
#include "ManualBase4Body.h"

/*********************************************************
* Member function Stub
*********************************************************/

/*********************************************************
* Global function Stub
*********************************************************/

/*********************************************************
* Get size of pointer to member function
*********************************************************/
class G__Sizep2memfuncManualBase4 {
 public:
  G__Sizep2memfuncManualBase4() {p=&G__Sizep2memfuncManualBase4::sizep2memfunc;}
    size_t sizep2memfunc() { return(sizeof(p)); }
  private:
    size_t (G__Sizep2memfuncManualBase4::*p)();
};

size_t G__get_sizep2memfuncManualBase4()
{
  G__Sizep2memfuncManualBase4 a;
  G__setsizep2memfunc((int)a.sizep2memfunc());
  return((size_t)a.sizep2memfunc());
}


/*********************************************************
* virtual base class offset calculation interface
*********************************************************/

   /* Setting up class inheritance */

/*********************************************************
* Inheritance information setup/
*********************************************************/
extern "C" void G__cpp_setup_inheritanceManualBase4() {

   /* Setting up class inheritance */
}

/*********************************************************
* typedef information setup/
*********************************************************/
extern "C" void G__cpp_setup_typetableManualBase4() {

   /* Setting up typedef entry */
   G__search_typename2("Int_t",105,-1,0,-1);
   G__setnewtype(-1,"Signed integer 4 bytes (int)",0);
   G__search_typename2("Option_t",99,-1,256,-1);
   G__setnewtype(-1,"Option string (const char)",0);
}

/*********************************************************
* Data Member information setup/
*********************************************************/

   /* Setting up class,struct,union tag member variable */
extern "C" void G__cpp_setup_memvarManualBase4() {
}
/***********************************************************
************************************************************
************************************************************
************************************************************
************************************************************
************************************************************
************************************************************
***********************************************************/

/*********************************************************
* Member function information setup for each class
*********************************************************/
static void G__setup_memfuncTDirectory(void) {
   /* TDirectory */
   G__tag_memfunc_setup(G__get_linked_tagnum(&G__ManualBase4LN_TDirectory));
   G__memfunc_setup("WriteObject",1122,G__ManualBase4_101_0_97, 105, -1, G__defined_typename("Int_t", 0), 0, 4, 1, 1, 0, 
"Y - - 10 - obj C - - 10 - name "
"C - 'Option_t' 10 '\"\"' option i - 'Int_t' 0 '0' bufsize", (char*)NULL, (void*) NULL, 0);
   G__memfunc_setup("GetObject",887,G__ManualBase4_101_0_98, 121, -1, -1, 0, 2, 1, 1, 0, 
"C - - 10 - namecycle Y - - 1 - ptr", (char*)NULL, (void*) NULL, 0);
   G__tag_memfunc_reset();
}


/*********************************************************
* Member function information setup
*********************************************************/
extern "C" void G__cpp_setup_memfuncManualBase4() {
}

/*********************************************************
* Global variable information setup for each class
*********************************************************/
static void G__cpp_setup_global0() {

   /* Setting up global variables */
   G__resetplocal();

}

static void G__cpp_setup_global1() {

   G__resetglobalenv();
}
extern "C" void G__cpp_setup_globalManualBase4() {
  G__cpp_setup_global0();
  G__cpp_setup_global1();
}

/*********************************************************
* Global function information setup for each class
*********************************************************/
static void G__cpp_setup_func0() {
   G__lastifuncposition();

}

static void G__cpp_setup_func1() {
}

static void G__cpp_setup_func2() {
   G__memfunc_setup("operator>>", 1000, G__ManualBase4__0_211, 117, G__get_linked_tagnum(&G__ManualBase4LN_TBuffer), -1, 1, 2, 1, 1, 0, 
"u 'TBuffer' - 1 - buf Y - - 1 - obj", (char*) NULL
, (void*) NULL, 0);
   G__memfunc_setup("operator<<", 996, G__ManualBase4__0_212, 117, G__get_linked_tagnum(&G__ManualBase4LN_TBuffer), -1, 1, 2, 1, 1, 0, 
"u 'TBuffer' - 1 - buf Y - - 10 - obj", (char*) NULL
, (void*) NULL, 0);

   G__resetifuncposition();
}

extern "C" void G__cpp_setup_funcManualBase4() {
  G__cpp_setup_func0();
  G__cpp_setup_func1();
  G__cpp_setup_func2();
}

/*********************************************************
* Class,struct,union,enum tag information setup
*********************************************************/
/* Setup class/struct taginfo */
G__linked_taginfo G__ManualBase4LN_TBuffer = { "TBuffer" , 99 , -1 };
G__linked_taginfo G__ManualBase4LN_TDirectory = { "TDirectory" , 99 , -1 };

/* Reset class/struct taginfo */
extern "C" void G__cpp_reset_tagtableManualBase4() {
  G__ManualBase4LN_TBuffer.tagnum = -1 ;
  G__ManualBase4LN_TDirectory.tagnum = -1 ;
}


extern "C" void G__cpp_setup_tagtableManualBase4() {

   /* Setting up class,struct,union tag entry */
   G__get_linked_tagnum(&G__ManualBase4LN_TBuffer);
   G__tagtable_setup(G__get_linked_tagnum(&G__ManualBase4LN_TDirectory),sizeof(TDirectory),-1,65280,"Describe directory structure in memory",NULL,G__setup_memfuncTDirectory);
}
extern "C" void G__cpp_setupManualBase4(void) {
  G__check_setup_version(30051515,"G__cpp_setupManualBase4()");
  G__set_cpp_environmentManualBase4();
  G__cpp_setup_tagtableManualBase4();

  G__cpp_setup_inheritanceManualBase4();

  G__cpp_setup_typetableManualBase4();

  G__cpp_setup_memvarManualBase4();

  G__cpp_setup_memfuncManualBase4();
  G__cpp_setup_globalManualBase4();
  G__cpp_setup_funcManualBase4();

   if(0==G__getsizep2memfunc()) G__get_sizep2memfuncManualBase4();
  return;
}
class G__cpp_setup_initManualBase4 {
  public:
    G__cpp_setup_initManualBase4() { G__add_setup_func("ManualBase4",(G__incsetup)(&G__cpp_setupManualBase4)); G__call_setup_funcs(); }
   ~G__cpp_setup_initManualBase4() { G__remove_setup_func("ManualBase4"); }
};
G__cpp_setup_initManualBase4 G__cpp_setup_initializerManualBase4;

