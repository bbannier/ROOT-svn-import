//
// File generated by c:\Devel\root_working\code\root.vc8\utils\src\rootcint_tmp.exe at Wed May 16 23:07:42 2007

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
#include "ManualTree2.h"

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
* tree/src/ManualTree2.cxx
* CAUTION: DON'T CHANGE THIS FILE. THIS FILE IS AUTOMATICALLY GENERATED
*          FROM HEADER FILES LISTED IN G__setup_cpp_environmentXXX().
*          CHANGE THOSE HEADER FILES AND REGENERATE THIS FILE.
********************************************************/

#ifdef G__MEMTEST
#undef malloc
#undef free
#endif

extern "C" void G__cpp_reset_tagtableManualTree2();

extern "C" void G__set_cpp_environmentManualTree2() {
   G__add_compiledheader("TTree.h");
   G__cpp_reset_tagtableManualTree2();
}
#include <new>
extern "C" int G__cpp_dllrevManualTree2() { return(30051515); }

/*********************************************************
* Member function Interface Method
*********************************************************/

/* TTree */
#include "ManualTree2Body.h"

/* Setting up global function */

/*********************************************************
* Member function Stub
*********************************************************/

/*********************************************************
* Global function Stub
*********************************************************/

/*********************************************************
* Get size of pointer to member function
*********************************************************/
class G__Sizep2memfuncManualTree2 {
public:
   G__Sizep2memfuncManualTree2(): p(&G__Sizep2memfuncManualTree2::sizep2memfunc) {}
   size_t sizep2memfunc() { return(sizeof(p)); }
private:
   size_t (G__Sizep2memfuncManualTree2::*p)();
};

size_t G__get_sizep2memfuncManualTree2()
{
   G__Sizep2memfuncManualTree2 a;
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
extern "C" void G__cpp_setup_inheritanceManualTree2() {

   /* Setting up class inheritance */
}

/*********************************************************
* typedef information setup/
*********************************************************/
extern "C" void G__cpp_setup_typetableManualTree2() {

   /* Setting up typedef entry */
   G__search_typename2("Int_t",105,-1,0,-1);
   G__setnewtype(-1,"Signed integer 4 bytes (int)",0);
   G__search_typename2("Option_t",99,-1,256,-1);
   G__setnewtype(-1,"Option string (const char)",0);
   G__search_typename2("Long64_t",110,-1,0,-1);
   G__setnewtype(-1,"Portable signed long integer 8 bytes",0);
}

/*********************************************************
* Data Member information setup/
*********************************************************/

/* Setting up class,struct,union tag member variable */
extern "C" void G__cpp_setup_memvarManualTree2() {
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
static void G__setup_memfuncTTree(void) {
   /* TTree */
   G__tag_memfunc_setup(G__get_linked_tagnum(&G__ManualTree2LN_TTree));
   G__memfunc_setup("Process",735,G__ManualTree2_126_0_132, 110, -1, G__defined_typename("Long64_t"), 0, 4, 1, 1, 0, 
      "Y - - 0 - selector C - 'Option_t' 10 '\"\"' option "
      "n - 'Long64_t' 0 '1000000000' nentries n - 'Long64_t' 0 '0' firstentry", (char*)NULL, (void*) NULL, 1);
   G__memfunc_setup("Branch",590,G__ManualTree2_126_0_187, 85, G__get_linked_tagnum(&G__ManualTree2LN_TBranch), -1, 0, 5, 1, 1, 0, 
      "C - - 10 - name C - - 10 - classname "
      "Y - - 3 - addobj i - 'Int_t' 0 '32000' bufsize "
      "i - 'Int_t' 0 '99' splitlevel", (char*)NULL, (void*) NULL, 0);
   G__memfunc_setup("Branch",590,G__ManualTree2_126_0_188, 85, G__get_linked_tagnum(&G__ManualTree2LN_TBranch), -1, 0, 4, 1, 1, 0, 
      "C - - 10 - name Y - - 3 - addobj "
      "i - 'Int_t' 0 '32000' bufsize i - 'Int_t' 0 '99' splitlevel", (char*)NULL, (void*) NULL, 0);
   G__memfunc_setup("SetBranchAddress",1600,G__ManualTree2_126_0_190, 121, -1, -1, 0, 3, 1, 1, 0, 
      "C - - 10 - bname Y - - 2 - add "
      "U 'TBranch' - 2 '0' ptr", (char*)NULL, (void*) NULL, 0);
   G__tag_memfunc_reset();
}


/*********************************************************
* Member function information setup
*********************************************************/
extern "C" void G__cpp_setup_memfuncManualTree2() {
}

/*********************************************************
* Global variable information setup for each class
*********************************************************/
static void G__cpp_setup_global0() {

   /* Setting up global variables */
   G__resetplocal();

}

static void G__cpp_setup_global1() {
}

static void G__cpp_setup_global2() {

   G__resetglobalenv();
}
extern "C" void G__cpp_setup_globalManualTree2() {
   G__cpp_setup_global0();
   G__cpp_setup_global1();
   G__cpp_setup_global2();
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

   G__resetifuncposition();
}

extern "C" void G__cpp_setup_funcManualTree2() {
   G__cpp_setup_func0();
   G__cpp_setup_func1();
   G__cpp_setup_func2();
}

/*********************************************************
* Class,struct,union,enum tag information setup
*********************************************************/
/* Setup class/struct taginfo */
G__linked_taginfo G__ManualTree2LN_TTree = { "TTree" , 99 , -1 };
G__linked_taginfo G__ManualTree2LN_TBranch = { "TBranch" , 99 , -1 };
G__linked_taginfo G__ManualTree2LN_TSelector = { "TSelector" , 99 , -1 };

/* Reset class/struct taginfo */
extern "C" void G__cpp_reset_tagtableManualTree2() {
   G__ManualTree2LN_TTree.tagnum = -1 ;
   G__ManualTree2LN_TBranch.tagnum = -1 ;
   G__ManualTree2LN_TSelector.tagnum = -1 ;
}


extern "C" void G__cpp_setup_tagtableManualTree2() {

   /* Setting up class,struct,union tag entry */
   G__tagtable_setup(G__get_linked_tagnum(&G__ManualTree2LN_TTree),sizeof(TTree),-1,65280,"Tree descriptor (the main ROOT I/O class)",NULL,G__setup_memfuncTTree);
   G__get_linked_tagnum_fwd(&G__ManualTree2LN_TBranch);
   G__get_linked_tagnum_fwd(&G__ManualTree2LN_TSelector);
}
extern "C" void G__cpp_setupManualTree2(void) {
   G__check_setup_version(30051515,"G__cpp_setupManualTree2()");
   G__set_cpp_environmentManualTree2();
   G__cpp_setup_tagtableManualTree2();

   G__cpp_setup_inheritanceManualTree2();

   G__cpp_setup_typetableManualTree2();

   G__cpp_setup_memvarManualTree2();

   G__cpp_setup_memfuncManualTree2();
   G__cpp_setup_globalManualTree2();
   G__cpp_setup_funcManualTree2();

   if(0==G__getsizep2memfunc()) G__get_sizep2memfuncManualTree2();
   return;
}
class G__cpp_setup_initManualTree2 {
public:
   G__cpp_setup_initManualTree2() { G__add_setup_func("ManualTree2",(G__incsetup)(&G__cpp_setupManualTree2)); G__call_setup_funcs(); }
   ~G__cpp_setup_initManualTree2() { G__remove_setup_func("ManualTree2"); }
};
G__cpp_setup_initManualTree2 G__cpp_setup_initializerManualTree2;

