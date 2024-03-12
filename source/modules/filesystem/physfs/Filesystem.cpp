#include "common/Console.hpp"

#include "modules/filesystem/physfs/Filesystem.hpp"

#include <filesystem>
#include <physfs.h>

#include <limits.h>
#include <string.h>

#define APPDATA_FOLDER ""
#define APPDATA_PREFIX ""
#define PATH_SEPARATOR "/"

#define normalize(x) std::string(std::filesystem::path((x)).lexically_normal())
#define parentize(x) std::string(std::filesystem::path((x)).parent_path())

static std::string getApplicationPath(std::string origin)
{
    if (origin == "embedded boot.lua")
    {
        switch (love::Console::Current)
        {
            case love::Console::CTR:
                return "sdmc:/lovepotion.3dsx";
            case love::Console::HAC:
                return "sdmc:/lovepotion.nro";
            case love::Console::CAFE:
                return "fs:/vol/external01/wiiu/apps/lovepotion/lovepotion.wuhb";
            default:
                return std::string {};
        }
    }

#if defined(__WIIU__)
    OSDynLoad_Module module;
    const auto success = 0;
    const auto name    = "RL_GetPathOfRunningExecutable";

    if (OSDynLoad_Acquire("homebrew_rpx_loader", &module) == OS_DYNLOAD_OK)
    {
        char path[256];

        bool (*RL_GetPathOfRunningExecutable)(char*, uint32_t);
        auto** ptrFunction = reinterpret_cast<void**>(&RL_GetPathOfRunningExecutable);

        // clang-format off
        if (OSDynLoad_FindExport(module, OS_DYNLOAD_EXPORT_FUNC, name, ptrFunction) == OS_DYNLOAD_OK)
        {
            if (RL_GetPathOfRunningExecutable(path, sizeof(path)) == success)
                return path;
        }
        // clang-format on
    }
    return std::string {};
#else
    return origin;
#endif
}

#include <utility/logfile.hpp>

namespace love
{
    // clang-format off
    static constexpr std::array<Filesystem::CommonPath, 2> appCommonPaths =
    {
        Filesystem::COMMONPATH_APP_SAVEDIR,
        Filesystem::COMMONPATH_APP_DOCUMENTS
    };
    // clang-format on

    static bool isAppCommonPath(Filesystem::CommonPath path)
    {
        switch (path)
        {
            case Filesystem::COMMONPATH_APP_SAVEDIR:
            case Filesystem::COMMONPATH_APP_DOCUMENTS:
                return true;
            default:
                return false;
        }
    }

    Filesystem::Filesystem() :
        FilesystemBase("love.filesystem.physfs"),
        appendIdentityToPath(false),
        fused(false),
        fusedSet(false),
        allowedPaths(),
        fullPaths(),
        commonPathMountInfo(),
        saveDirectoryNeedsMounting(false)
    {
        this->requirePath = { "?.lua", "?/init.lua" };
    }

    Filesystem::~Filesystem()
    {
        if (PHYSFS_isInit())
            PHYSFS_deinit();
    }

    const char* Filesystem::getLastError()
    {
        return PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
    }

    void Filesystem::init(const char* arg0)
    {
        this->executablePath = getApplicationPath(arg0);

        if (this->executablePath.empty())
            throw love::Exception("Error getting application path.");

        if (!PHYSFS_init(this->executablePath.c_str()))
            throw love::Exception("Error initializing PhysFS: {:s}", Filesystem::getLastError());

        PHYSFS_setWriteDir(nullptr);
        this->setSymlinksEnabled(true);
    }

    void Filesystem::setFused(bool enable)
    {
        if (this->fusedSet)
            return;

        this->fused    = enable;
        this->fusedSet = true;
    }

    bool Filesystem::isFused() const
    {
        if (!this->fusedSet)
            return false;

        return this->fused;
    }

    bool Filesystem::setupWriteDirectory()
    {
        if (!PHYSFS_isInit())
            return false;

        if (!this->saveDirectoryNeedsMounting)
            return true;

        if (this->saveIdentity.empty())
            return false;

        bool create      = true;
        auto path        = COMMONPATH_APP_SAVEDIR;
        auto permissions = MOUNT_PERMISSIONS_READWRITE;

        // clang-format off
        if (!this->mountCommonPathInternal(path, nullptr, permissions, this->appendIdentityToPath, create))
            return false;
        // clang-format on

        this->saveDirectoryNeedsMounting = false;

        return true;
    }

    bool Filesystem::setIdentity(std::string_view identity, bool appendToPath)
    {
        if (!PHYSFS_isInit())
            return false;

        if (identity.empty())
            return false;

        for (auto& path : appCommonPaths)
        {
            if (!this->commonPathMountInfo[path].mounted)
                continue;

            std::string fullpath = this->getFullCommonPath(path);
            if (!fullpath.empty() && !PHYSFS_unmount(fullpath.data()))
                return false;
        }

        std::array<bool, COMMONPATH_MAX_ENUM> oldPaths { false };

        for (auto& path : appCommonPaths)
        {
            oldPaths[path] = this->commonPathMountInfo[path].mounted;

            if (commonPathMountInfo[path].mounted)
                this->unmount(path);
        }

        for (CommonPath path : appCommonPaths)
            this->fullPaths[path].clear();

        this->saveIdentity         = identity;
        this->appendIdentityToPath = appendToPath;

        // clang-format off
        if (!this->mountCommonPathInternal(COMMONPATH_APP_SAVEDIR, nullptr, MOUNT_PERMISSIONS_READWRITE, appendToPath, false))
            this->saveDirectoryNeedsMounting = true;

        for (CommonPath path : appCommonPaths)
        {
            if (oldPaths[path] && path != COMMONPATH_APP_SAVEDIR)
            {
                auto info = this->commonPathMountInfo[path];
                this->mountCommonPathInternal(path, info.mountPoint.c_str(), info.permissions, appendToPath, true);
            }
        }
        // clang-format on
        return true;
    }

    std::string Filesystem::getIdentity() const
    {
        return this->saveIdentity;
    }

    bool Filesystem::setSource(const char* source)
    {
        if (!PHYSFS_isInit())
            return false;

        if (!this->source.empty())
            return false;

        if (!PHYSFS_mount(source, nullptr, 1))
            return false;

        this->source = source;

        return true;
    }

    std::string Filesystem::getSource() const
    {
        return this->source;
    }

    bool Filesystem::mount(const char* archive, const char* mountpoint, bool appendtoPath)
    {
        if (!PHYSFS_isInit() || !archive)
            return false;

        std::string realPath {};
        std::string sourceBase = this->getSourceBaseDirectory();

        auto iterator = std::find(this->allowedPaths.begin(), this->allowedPaths.end(), archive);

        if (iterator != this->allowedPaths.end())
            realPath = *iterator;
        else if (this->isFused() && sourceBase.compare(archive))
            realPath = sourceBase;
        else
        {
            if (strlen(archive) == 0 || strstr(archive, "..") || strcmp(archive, "/") == 0)
                return false;

            const char* realDirectory = PHYSFS_getRealDir(archive);
            if (!realDirectory)
                return false;

            realPath = realDirectory;

            if (realPath.find(this->source) == 0)
                return false;

            realPath += PATH_SEPARATOR;
            realPath += archive;
        }

        return this->mountFullPath(realPath.c_str(), mountpoint, MOUNT_PERMISSIONS_READ,
                                   appendtoPath);
    }

    bool Filesystem::mount(Data* data, const char* archive, const char* mountpoint,
                           bool appendToPath)
    {
        if (!PHYSFS_isInit() || !archive)
            return false;

        if (PHYSFS_mountMemory(data->getData(), data->getSize(), nullptr, archive, mountpoint,
                               appendToPath))
        {
            this->mountedData[archive] = data;
            return true;
        }

        return false;
    }

    bool Filesystem::mountFullPath(const char* archive, const char* mountpoint,
                                   MountPermissions permissions, bool appendToPath)
    {
        if (!PHYSFS_isInit() || !archive)
            return false;

        if (permissions == MOUNT_PERMISSIONS_READWRITE)
        {
            if (!PHYSFS_setWriteDir(archive))
                return false;
        }

        return PHYSFS_mount(archive, mountpoint, appendToPath) != 0;
    }

    bool Filesystem::mountCommonPathInternal(CommonPath path, const char* mountPoint,
                                             MountPermissions permissions, bool appendToPath,
                                             bool create)
    {
        std::string fullpath = this->getFullCommonPath(path);

        if (fullpath.empty())
            return false;

        if (create && isAppCommonPath(path) && !this->isRealDirectory(fullpath))
        {
            if (!this->createRealDirectory(fullpath))
                return false;
        }

        if (this->mountFullPath(fullpath.c_str(), mountPoint, permissions, appendToPath))
        {
            std::string point               = mountPoint != nullptr ? mountPoint : "/";
            this->commonPathMountInfo[path] = { true, point, permissions };

            return true;
        }

        return false;
    }

    bool Filesystem::mountCommonPath(CommonPath path, const char* mountPoint,
                                     MountPermissions permissions, bool appendToPath)
    {
        return this->mountCommonPathInternal(path, mountPoint, permissions, appendToPath, true);
    }

    bool Filesystem::unmount(const char* archive)
    {
        if (!PHYSFS_isInit() || !archive)
            return false;

        auto dataIterator = this->mountedData.find(archive);
        if (dataIterator != this->mountedData.end() && PHYSFS_unmount(archive) != 0)
        {
            this->mountedData.erase(dataIterator);
            return true;
        }

        auto iterator = std::find(this->allowedPaths.begin(), this->allowedPaths.end(), archive);
        if (iterator != this->allowedPaths.end())
            return this->unmountFullPath(archive);

        std::string sourceBase = this->getSourceBaseDirectory();
        if (this->isFused() && sourceBase.compare(archive))
            return this->unmountFullPath(archive);

        if (strlen(archive) == 0 || strstr(archive, "..") || strcmp(archive, "/") == 0)
            return false;

        const char* realDirectory = PHYSFS_getRealDir(archive);
        if (!realDirectory)
            return false;

        std::string realPath = realDirectory;
        realPath += PATH_SEPARATOR;
        realPath += archive;

        if (PHYSFS_getMountPoint(realPath.c_str()) == nullptr)
            return false;

        return PHYSFS_unmount(realPath.c_str()) != 0;
    }

    bool Filesystem::unmount(Data* data)
    {
        for (const auto& dataPair : this->mountedData)
        {
            if (dataPair.second.get() == data)
            {
                std::string archive = dataPair.first;
                return this->unmount(archive.c_str());
            }
        }

        return false;
    }

    bool Filesystem::unmount(CommonPath path)
    {
        std::string fullpath = this->getFullCommonPath(path);

        if (!fullpath.empty() && this->unmountFullPath(fullpath.c_str()))
        {
            this->commonPathMountInfo[path].mounted = false;
            return true;
        }

        return false;
    }

    bool Filesystem::unmountFullPath(const char* fullpath)
    {
        if (!PHYSFS_isInit() || !fullpath)
            return false;

        return PHYSFS_unmount(fullpath) != 0;
    }

    File* Filesystem::openFile(std::string_view filename, File::Mode mode) const
    {
        return new File(filename, mode);
    }

    std::string Filesystem::getFullCommonPath(CommonPath path)
    {
        if (!this->fullPaths[path].empty())
            return this->fullPaths[path];

        if (isAppCommonPath(path))
        {
            if (this->saveIdentity.empty())
                return this->fullPaths[path];

            std::string root {};

            switch (path)
            {
                case COMMONPATH_APP_SAVEDIR:
                {
                    root = this->getFullCommonPath(COMMONPATH_USER_APPDATA);
                    break;
                }
                case COMMONPATH_APP_DOCUMENTS:
                {
                    root = this->getFullCommonPath(COMMONPATH_USER_DOCUMENTS);
                    break;
                }
                default:
                    break;
            }

            if (root.empty())
                return this->fullPaths[path];

            std::string suffix {};

            if (this->isFused())
                suffix = PATH_SEPARATOR + this->saveIdentity;
            else
                suffix = (PATH_SEPARATOR APPDATA_FOLDER PATH_SEPARATOR) + this->saveIdentity;

            this->fullPaths[path] = normalize(root + suffix);
            return this->fullPaths[path];
        }

        switch (path)
        {
            case COMMONPATH_APP_SAVEDIR:
            case COMMONPATH_APP_DOCUMENTS:
                break;
            case COMMONPATH_USER_HOME:
            {
                this->fullPaths[path] = normalize(PHYSFS_getUserDir());
                break;
            }
            case COMMONPATH_USER_APPDATA:
            {
                std::string fullpath {};

                if (Console::is(Console::CAFE))
                    fullpath = parentize(this->executablePath);
                else
                    fullpath = PHYSFS_getUserDir();

                this->fullPaths[path] = normalize(fullpath + "/save/");

                break;
            }
            case COMMONPATH_USER_DOCUMENTS:
            {
                this->fullPaths[path] = normalize(PHYSFS_getUserDir());
                break;
            }
            case COMMONPATH_MAX_ENUM:
            default:
                break;
        }

        return this->fullPaths[path];
    }

    std::string Filesystem::getWorkingDirectory()
    {
        if (!this->currentDirectory.empty())
            return this->currentDirectory;

        this->currentDirectory = std::filesystem::current_path().string();

        if (Console::is(Console::CAFE))
            this->currentDirectory = parentize(getApplicationPath(""));

        return this->currentDirectory;
    }

    std::string Filesystem::getUserDirectory()
    {
        return this->getFullCommonPath(COMMONPATH_USER_HOME);
    }

    std::string Filesystem::getAppdataDirectory()
    {
        return this->getFullCommonPath(COMMONPATH_USER_APPDATA);
    }

    std::string Filesystem::getSaveDirectory()
    {
        return this->getFullCommonPath(COMMONPATH_APP_SAVEDIR);
    }

    std::string Filesystem::getSourceBaseDirectory()
    {
        const auto path = std::filesystem::path(this->source);

        if (!path.has_parent_path())
            return std::string {};

        return path.parent_path().string();
    }

    std::string Filesystem::getRealDirectory(const char* filename) const
    {
        if (!PHYSFS_isInit())
            throw love::Exception(E_PHYSFS_NOT_INITIALIZED);

        const char* resolved = PHYSFS_getRealDir(filename);

        if (resolved == nullptr)
            throw love::Exception("File does not exist on disk.");

        return std::string(resolved);
    }

    bool Filesystem::exists(const char* filepath) const
    {
        if (!PHYSFS_isInit())
            return false;

        return PHYSFS_exists(filepath) != 0;
    }

    bool Filesystem::getInfo(const char* filepath, Info& info) const
    {
        if (!PHYSFS_isInit())
            return false;

        PHYSFS_Stat stat {};
        if (!PHYSFS_stat(filepath, &stat))
            return false;

        info.modtime  = (int64_t)stat.modtime;
        info.size     = (int64_t)stat.filesize;
        info.readonly = (stat.readonly != 0);

        if (stat.filetype == PHYSFS_FILETYPE_REGULAR)
            info.type = FILETYPE_FILE;
        else if (stat.filetype == PHYSFS_FILETYPE_DIRECTORY)
            info.type = FILETYPE_DIRECTORY;
        else if (stat.filetype == PHYSFS_FILETYPE_SYMLINK)
            info.type = FILETYPE_SYMLINK;
        else
            info.type = FILETYPE_OTHER;

        return true;
    }

    bool Filesystem::createDirectory(const char* path)
    {
        if (!PHYSFS_isInit())
            return false;

        if (!this->setupWriteDirectory())
            return false;

        if (!PHYSFS_mkdir(path))
            return false;

        return true;
    }

    bool Filesystem::remove(const char* filename)
    {
        if (!PHYSFS_isInit())
            return false;

        if (!this->setupWriteDirectory())
            return false;

        if (!PHYSFS_delete(filename))
            return false;

        return true;
    }

    FileData* Filesystem::read(std::string_view filename, int64_t size) const
    {
        File file(filename, File::MODE_READ);

        return file.read(size);
    }

    FileData* Filesystem::read(std::string_view filename) const
    {
        File file(filename, File::MODE_READ);

        return file.read();
    }

    void Filesystem::write(std::string_view filename, const void* data, int64_t size) const
    {
        File file(filename, File::MODE_WRITE);

        if (!file.write(data, size))
            throw love::Exception(E_DATA_NOT_WRITTEN);
    }

    void Filesystem::append(std::string_view filename, const void* data, int64_t size) const
    {
        File file(filename, File::MODE_APPEND);

        if (!file.write(data, size))
            throw love::Exception(E_DATA_NOT_WRITTEN);
    }

    bool Filesystem::getDirectoryItems(const char* directory, std::vector<std::string>& items)
    {
        if (!PHYSFS_isInit())
            return false;

        char** result = PHYSFS_enumerateFiles(directory);

        if (result == nullptr)
            return false;

        for (char** i = result; *i != 0; i++)
            items.push_back(*i);

        PHYSFS_freeList(result);
        return true;
    }

    void Filesystem::setSymlinksEnabled(bool enable)
    {
        if (!PHYSFS_isInit())
            return;

        PHYSFS_permitSymbolicLinks(enable);
    }

    bool Filesystem::areSymlinksEnabled() const
    {
        if (!PHYSFS_isInit())
            return false;

        return PHYSFS_symbolicLinksPermitted() != 0;
    }

    std::vector<std::string>& Filesystem::getRequirePath()
    {
        return this->requirePath;
    }
} // namespace love
