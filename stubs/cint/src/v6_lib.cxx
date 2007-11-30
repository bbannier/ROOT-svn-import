/* /% C %/ */
/***********************************************************************
 * cint (C/C++ interpreter)
 ************************************************************************
 * Source file v6_lib.cxx
 * Leandro Franco
 ************************************************************************
 * Description:
 * Assuming the introduction of the mangled name in G__memfunc_setup2
 * now we need a way to fetch the address of that name.
 * This is normally done using:
 *    void *dlsym(void *restrict handle, const char *restrict name);
 * 
 * but we don't have the handle of the library. This file was created
 * for that (and will probably be called from G__stub_method_calling)
 ************************************************************************
 * Copyright(c) 1995~2004  Masaharu Goto
 *
 * For the licensing terms see the file COPYING
 *
 ************************************************************************/

#include "common.h"
#include "Api.h"

//#include <cxxabi.h>
#include <dlfcn.h>
#include <link.h>
//#include <iostream>
//#include <fstream>
//#include <vector>
#include <list>
#include <limits.h>

//#include <set>
//#include <string>
//#include <map>
using namespace std;

//______________________________________________________________________________
static struct link_map *
locate_linkmap()
{
   /* locate base link-map in memory */
   //
   // LF: 08/05/07
   // Taken from the phrack magazine but modified to find the link map
   // of the current process (as such we dont need ptracing abilities)

   Elf32_Ehdr      *ehdr   = (Elf32_Ehdr*) malloc(sizeof(Elf32_Ehdr));
   Elf32_Phdr      *phdr   = (Elf32_Phdr*) malloc(sizeof(Elf32_Phdr));
   Elf32_Dyn       *dyn    = (Elf32_Dyn*)  malloc(sizeof(Elf32_Dyn));
   Elf32_Word      got;
   struct link_map *l      = (link_map*)   malloc(sizeof(struct link_map));
   unsigned long   phdr_addr , dyn_addr , map_addr;


   /* first we check from elf header, mapped at 0x08048000, the offset
    * to the program header table from where we try to locate
    * PT_DYNAMIC section.
    */
   memcpy(ehdr, (const void*)0x08048000, sizeof(Elf32_Ehdr));

   phdr_addr = 0x08048000 + ehdr->e_phoff;

   memcpy(phdr, (const void*)phdr_addr, sizeof(Elf32_Phdr));

   while ( phdr->p_type != PT_DYNAMIC ) {
      memcpy(phdr, (const void*) (phdr_addr += sizeof(Elf32_Phdr)), sizeof(Elf32_Phdr));
   }

   /* now go through dynamic section until we find address of the GOT
    */

   memcpy(dyn, (const void*)phdr->p_vaddr, sizeof(Elf32_Dyn));
   dyn_addr = phdr->p_vaddr;

   while ( dyn->d_tag != DT_PLTGOT ) {
      memcpy(dyn, (const void*)(dyn_addr += sizeof(Elf32_Dyn)), sizeof(Elf32_Dyn));
   }

   got = (Elf32_Word) dyn->d_un.d_ptr;
   got += 4; 		/* second GOT entry, remember? */

   /* now just read first link_map item and return it */
   memcpy(&map_addr, (const void*) got, 4);
   
   memcpy(l, (const void*)map_addr, sizeof(struct link_map));

   free(phdr);
   free(ehdr);
   free(dyn);

   return l;
}

//______________________________________________________________________________
static list<struct link_map *> *
get_linkmap_list()
{
   // Get the list of the libraries currently loaded.
   //
   // Normally, It is a single linked list but we need to traverse from
   // end to beginning so i'm creating a normal list for that
   list<struct link_map *> *linkmap_list = new list<struct link_map *>;

   struct link_map *map = locate_linkmap();
   struct link_map *map_iter;

   for (map_iter = map; map_iter; map_iter = map_iter->l_next) {
      if( map_iter->l_addr ) {
         // push them at the beginning of the list to get them in
         // the rigth order later
         linkmap_list->push_front(map_iter);
      }
   }

   return linkmap_list;
}

//______________________________________________________________________________
void*
G__get_address(const char* mangled_name)
{
   // Get the addrees of a mangled name.
   // since we dont know in which library we wpuld have to look
   // in all the libraries that have already been loaded
   
    void *address=0;
   
//    // rem to delete this list
//    list<struct link_map *> *linkmap_list = get_linkmap_list();

//    list<struct link_map *>::iterator iter;
//    for (iter = linkmap_list->begin(); iter != linkmap_list->end(); ++iter) {
//       struct link_map *maptmp = *iter;
      
//       int ok = dlinfo(RTLD_SELF, RTLD_DI_LINKMAP, &maptmp);
//       if (!ok&&!strcmp(maptmp->l_name,libname)) {
//          address = dlsym(maptmp->l_addr,mangled_name);
//          break;
//       }
  
//    }

    address = dlsym(0,mangled_name);
    for (int i=0;(i<G__allsl)&&(!address);i++){
       address = dlsym(G__sl_handle[i],mangled_name);       
    }

   // if (!address)
   //address = dlsym(RTLD_DEFAULT, mangled_name);

  //   int i=0;

//     if (libname){

//        while(i<G__nfile) {

//           if(G__matchfilename(i,libname))
//              break;

//           ++i;
//        }      
//     }
    //   address = dlsym(G__sl_handle[G__srcfile[i].dictpos->allsl],mangled_name);
    
    if (!address)
       address = dlsym(RTLD_DEFAULT, mangled_name);
   
   //delete linkmap_list;
   return address;
}

