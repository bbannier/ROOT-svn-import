
#ifndef G__DICTIONARY
#error RtypesImp.h should only be included by ROOT dictionaries.
#endif

inline const R__InitBehavior *R__DefineBehavior(void * /*parent_type*/,
                                                void * /*actual_type*/)
{
   return new R__DefaultInitBehavior();
}

template <class T> const R__InitBehavior *R__tInit<T>::fgAction       = 0;
template <class T> TClass                *R__tInit<T>::fgClass        = 0;
template <class T> const char            *R__tInit<T>::fgImplFileName = 0;
template <class T> Int_t                  R__tInit<T>::fgImplFileLine = 0;


