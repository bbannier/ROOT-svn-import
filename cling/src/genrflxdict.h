
/* dict.h */

#ifndef __CLR_DICT_H__
#define __CLR_DICT_H__

#include "Reflex/Reflex.h"
#include "Reflex/DictionaryGenerator.h"

/* ---------------------------------------------------------------------- */

void GlobalScope_GenerateDict(Reflex::DictionaryGenerator& generator);

/* ---------------------------------------------------------------------- */

class TContext;
TContext* GetFirstContext ();
void DeleteContexts ();

/* ---------------------------------------------------------------------- */

#endif /* __CLR_DICT_H__ */
