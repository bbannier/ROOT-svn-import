#include <vector>

namespace Cint {
   class FileRepository;

   class FileEntry {
   public:
      typedef long ID_t;
      typedef void* Lock_t;

      class FullyQualifiedPath: public std::string {
      public:
         operator bool() { return !empty(); }
      protected:
         FullyQualifiedPath(std::string&);
         friend class FileRepository;

      private:
         FullyQualifiedPath();
      };

      // Reference to a file entry, safe for deletion (in contrast to FileEntry*)
      typedef Reflex::Container<std::string, FileEntry*>::const_iterator Ref_t;

      // Representation of a file.
      FileEntry(const FullyQualifiedPath& filename, const FileRepository* repo);
      ~FileEntry();

      // Fully qualified name (c.f. Which())
      const FullyQualifiedPath& Name() const;

      // Whether this file can be unloaded
      bool IsLocked() const;
      bool Lock(Lock_t locked_by) const;
      const Reflex::Collection<Lock_t>& GetLocks() const;

      // Whether it can be theoretically unloaded
      bool IsPermanent();

      // Unload the file
      bool Unload();

      bool IsSharedLibrary() const;
      bool IsSourceFile() const;

      // handle as returned by dlopen / ...
      ShLibHandle_t* GetSharedLibHandle() const;

      // unique ID for the file (e.g. inode) - same physical files have the same ID
      ID_t PhysicalUniqueId() const { return fId; }

      // Add a direct dependency of this FileEntry, stating:
      // if this gets unloaded, fe will need to get unloaded, too.
      void AddDependency(const FileEntry* fe);
      // Get direct dependencies as set by AddDependency
      const std::vector<const FileEntry*>& GetDependencies() const;
      // Collect all dependencies, recursing over the dependencies' dependencies
      vector::set<const FileEntry*> GetAllDependencies() const;

      // Get files directly depending on this
      const std::vector<const FileEntry*>& GetDependents() const;
      // Get files directly or indirectly depending on this
      vector::set<const FileEntry*> GetAllDependents() const;

      typedef void* UnloadCallbackHandle_t;
      // Register void callback(Interpreter&, const FileEntry&) to call when unloading.
      // Callbacks will be called in reverse order of registration ("filo")
      UnloadCallbackHandle_t RegisterUnloadCallback(const Reflex::Callback<void, Interpreter&, const FileEntry&>& callback);

      // Unregister callback to call when unloading
      bool UnregisterUnloadCallback(UnloadCallbackHandle_t handle);
   };

   class FileRepository {
   public:
      enum EError {
         kSuccess,
         kFileNotFound,
         kFileLocked,
      };
      enum EPath {
         kInclude,
         kMacro,
         kLibrary,
         kExecutable
      };

      FileRepository();

      // return the per-process instance, holding shared libraries and
      // publicly accessible files
      static FileRepository& StaticInstance();

      // Set the search paths for sources, includes, and shared libraries
      void SetPath(EPath which, std::vector<std::string> inclpath);
      void AddPath(EPath which, const char* inclpath);

      // Given a possibly unqualified filename, lookup an existing file and
      // return its fully qualified path. Return empty string in case of
      // failure.
      FullyQualifiedPath Which(const std::string& filename) const;
      FullyQualifiedPath Which(const char* filename) const;

      // Given a possibly unqualified filename, lookup an existing file and
      // load it. If the file is a symlink that has already been loaded as
      // another file with another name, return that otehr FileEntry.
      // Return 0 in case of failure.
      FileEntry* Load(const FullyQualifiedPath& filename);
      FileEntry* Load(const std::string& filename);
      FileEntry* Load(const char* filename);

      // Query the repository for a file that matches filename.
      // Symlinks are not followed.
      FileEntry* IsLoaded(const FullyQualifiedPath& filename);
      FileEntry* IsLoaded(const std::string& filename);
      FileEntry* IsLoaded(const char* filename);

      // Unload a file. Return error code or kSucces if successful.
      EError Unload(const FullyQualifiedPath& filename);
      EError Unload(const std::string& filename);
      EError Unload(const char* filename);
      EError Unload(const FileEntry* entry);

      typedef void* DependencyCallbackHandle_t;
      typedef void* PreLoadCallbackHandle_t;
      typedef void* PostLoadCallbackHandle_t;

      // Sequence:
      // if (DependencyHook()) if (PreLoadHook()) {Load(); PostLoadHook();}

      // Callback returns whether a dependency for laoding FileEntry cannot be satisfied.
      DependencyCallbackHandle_t RegisterDependencyCallback(const Reflex::Callback<bool, Interpreter&, const FileEntry&>& callback);
      bool UnregisterDependencyCallback(DependencyCallbackHandle_t handle);

      // Callback loads dependencies for laoding FileEntry; returns false for failure canceling load request.
      PreLoadCallbackHandle_t PreLoadCallback(const Reflex::Callback<bool, Interpreter&, const FileEntry&>& callback);
      bool UnregisterPreLoadCallback(PreLoadCallbackHandle_t handle);

      // Callback to inform that a FileEntry has been loaded.
      PostLoadCallbackHandle_t PostLoadCallback(const Reflex::Callback<void, Interpreter&, const FileEntry&>& callback);
      bool UnregisterPostLoadCallback(PostLoadCallbackHandle_t handle);
   };
}
