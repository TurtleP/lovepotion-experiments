#pragma once

#include "modules/filesystem/Filesystem.tcc"
#include "modules/filesystem/physfs/File.hpp"

namespace love
{
    class Filesystem final : public FilesystemBase<Filesystem>
    {
      public:
        Filesystem();

        virtual ~Filesystem();

        void init(const char* arg0);

        void setFused(bool fused);

        bool isFused() const;

        bool setupWriteDirectory();

        bool setIdentity(const std::string& identity, bool appendToPath = false);

        std::string getIdentity() const;

        bool setSource(const char* path);

        std::string getSource() const;

        // TODO: mounting
        bool mount(CommonPath path, const char* mountPoint, MountPermissions permissions,
                   bool appendToPath);

        bool mountCommonPath(CommonPath path, const char* mountPoint, MountPermissions permissions,
                             bool appendToPath);

        bool mountFullPath(const char* archive, const char* mountpoint,
                           MountPermissions permissions, bool appendToPath);

        bool unmountFullPath(const char* fullpath);

        bool unmount(CommonPath path);

        File* openFile(const char* filename, File::Mode mode) const;

        FileData* newFileData(const void* data, size_t size, const char* filename) const;

        FileData* read(const char* filename, int64_t size) const;

        FileData* read(const char* filename) const;

        void write(const char* filename, const void* data, int64_t size) const;

        void append(const char* filename, const void* data, int64_t size) const;

        std::string getFullCommonPath(CommonPath path);

        std::string getWorkingDirectory();

        std::string getUserDirectory();

        std::string getAppdataDirectory();

        std::string getSaveDirectory();

        std::string getSourceBaseDirectory();

        std::string getRealDirectory(const char* filename) const;

        bool exists(const char* filepath) const;

        bool getInfo(const char* filepath, Info& info) const;

        bool createDirectory(const char* path);

        bool remove(const char* filepath) const;

        bool getDirectoryItems(const char* path, std::vector<std::string_view>& items);

        void setSymlinksEnabled(bool enable);

        bool areSymlinksEnabled() const;

        std::vector<std::string>& getRequirePath();

        void allowMountingPath(const char* path);

        static const char* getLastError();

      private:
        struct CommonPathMountInfo
        {
            bool mounted;
            std::string mountPoint;
            MountPermissions permissions;
        };

        bool mountCommonPathInternal(CommonPath path, const char* mountPoint,
                                     MountPermissions permissions, bool appendToPath,
                                     bool createDirectory);

        std::string currentDirectory;

        std::string saveIdentity;
        bool appendIdentityToPath;

        std::string source;

        bool fused;
        bool fusedSet;

        std::vector<std::string> requirePath;
        std::vector<std::string_view> allowedMountPaths;

        std::array<std::string, COMMONPATH_MAX_ENUM> fullPaths;
        std::array<CommonPathMountInfo, COMMONPATH_MAX_ENUM> commonPathMountInfo;
        bool saveDirectoryNeedsMounting;
    };
} // namespace love
