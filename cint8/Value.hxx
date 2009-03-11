namespace Cint {

   // CINT's Value adds the ability to cast using user conversions.
   class Value: public Reflex::Value {
   public:
      // construct given the needed items
      Value(void* addr, Reflex::Type type, bool owned = false);

      // constructor shortcut: type from T, owned=false
      template <typename T>
      Value(T& v);

      // constructor shortcut: allocate memory internally, owned=true
      Value(Reflex::Type type)

      // Destructs object and deletes memory if owned.
      ~Value();

      // Cast the value to "to"; returns Reflex::Dummy::Value if not successful.
      // Also calls user conversion functions if available.
      Value Cast(Reflex::Type to) const;

      // Whether the memory "pointed to" by the value is owned by the object.
      bool OwnsMemory() const;
   }
}
