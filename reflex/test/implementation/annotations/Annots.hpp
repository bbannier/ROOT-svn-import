
#include <string>
#include <vector>

class Version {
public:
   std::string getMajor() const { return fMajor; }
   void setMajor(const std::string & value) { fMajor = value; }

private:
   std::string fMajor;
};

class Credits {
public:
   Credits();
   ~Credits();

   std::string getAuthor() const { return fAuthor; }
   void setAuthor(const std::string & value) { fAuthor = value; }

   const Version* getVersion() const { return fVersion; }
   void setVersion(Version* value) { fVersion = value; }

   const std::vector<std::string>& getReviewers() const { return fReviewers; }
   std::vector<std::string>& getReviewers() { return fReviewers; }

private:
   std::string fAuthor;
   const Version* fVersion;
   std::vector<std::string> fReviewers;
};


class Field {
public:
   std::string getName() const { return fName; }
   void setName(const std::string & value) { fName = value; }

private:
   std::string fName;
};

class MetaModel {
public:
   MetaModel();
   ~MetaModel();

   unsigned int getRevision() const { return fRevision; }
   void setRevision(unsigned int value) { fRevision = value; }

   const std::vector<const Field*>& getFields() const { return fFields; }
   std::vector<const Field*>& getFields() { return fFields; }

private:
   unsigned int fRevision;
   std::vector<const Field*> fFields;
};


class Getter {
};


/* @class ModelClass
 * @custom:Credits(Author="Philippe Bourgau",Version=@Version(Major="0.9"),Reviewers={"Me","You"})
 * @custom:MetaModel(Revision=1,Fields={@Field(Name="Surprise")})
 */
class ModelClass {
public:
   
   // @custom:Getter()
   std::string getSuprise() const { return "fake"; }
};




// Clean ups have been commented out because they triggered runtime error when loading libraries.
// Not a problem since we are just testing.

inline Credits::Credits() : fVersion(0) {
}

inline Credits::~Credits() {
//   delete fVersion;
}

inline MetaModel::MetaModel() : fRevision(0) {
}

inline MetaModel::~MetaModel() {
   //for(size_t i = 0; i < fFields.size(); ++i) {
   //   delete fFields[i];
   //}
}

