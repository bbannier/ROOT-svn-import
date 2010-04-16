class ClingDict {
public:
   ClingDict(const char* name,
             std::vector<const char*> parsefiles);

   void parse(CI*) {}
   virtual void select(CI*) {}
   virtual void generate() = 0;
};
