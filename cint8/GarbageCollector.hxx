namespace Cint {

   // The GarbageCollector counts references to addresses.
   //    float* f = new float[5]
   // under GarbageCollection control will add the return value of new
   // as an address to watch, with &f as one of the references to it.
   class GarbageCollector {
   public:
      // Delete unreferenced, garbage collected memory. Returns the number of deletions.
      long GarbageCollect();

      // Retrieve the set of watched (i.e. reference counted) addresses
      std::set<void*> WatchedAddresses() const;

      // Whether addr is under garbage collection control.
      bool IsWatched(void* addr);

      // Whether there are references to addr, == (bool)References(addr).size()
      bool IsReferenced(void* addr);

      // Retrieve the set of references for an address
      std::set<void*> References(void* addr);

      // Inform that ref references addr; allocate reference observation for addr if needed.
      // Returns false if ref was already known to reference addr
      bool AddRef(void* addr, void* ref);
      // Inform that ref is not referencing addr anymore. Returns false if
      // ref wasn't known to refer to addr to begin with.
      bool DelRef(void* addr, void* ref);
   };
}
