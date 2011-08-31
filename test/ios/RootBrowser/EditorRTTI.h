class TObject;

namespace ROOT_IOSBrowser {

enum {
   kLineEditor = 1,
   kFillEditor = 2,
   kPadEditor = 4
};

unsigned GetRequiredEditors(const TObject *obj);

}