namespace Cint {

   // CINT's Value adds the ability to cast using user conversions.
   class Value: public Reflex::Value {
   public:
      // Cast the value to "to"; returns Reflex::Dummy::Value if not successful.
      // Also calls user conversion functions if available.
      Value Cast(Reflex::Type to) const;
   }
}
