#include "MyController.h"
#include <efsw/efsw.hpp>
#include <mutex>
#include <optional>

std::string FixPath(std::string path)
{
  // Replace Windows-style separators
  std::transform(path.begin(), path.end(), path.begin(),
                 [](char c) { return c == '\\' ? '/' : c; });

  // Replace suplicating slashes
  while (1) {
    auto pos = path.find("//");
    if (pos == std::string::npos) {
      break;
    }
    path.replace(pos, 2, "/");
  }

  // Remove last slash (except for C:/)
  bool isDriveLetter = path.size() == 3 && path[1] == ':';
  if (!isDriveLetter) {
    if (!path.empty() && path.back() == '/') {
      path.pop_back();
    }
  }

  return path;
}

bool StartsWith(const std::string& str1, const std::string& str2)
{
  if (str1.size() < str2.size()) {
    return false;
  }
  if (memcmp(str1.data(), str2.data(), str2.size()) != 0) {
    return false;
  }
  return true;
}

class MyController::Impl : public efsw::FileWatchListener
{
public:
  void handleFileAction(efsw::WatchID watchid, const std::string& dir,
                        const std::string& filename, efsw::Action action,
                        std::string oldFilename) override
  {
    std::lock_guard l(searchTaskM);

    std::unique_ptr<DirectoryInfo> rootDirectoryInfo;
    searchTask->Stop(rootDirectoryInfo);

    std::cout << std::endl;
    std::cout << "dir=" << FixPath(dir) << std::endl;
    std::cout << "filename=" << filename << std::endl;
    std::cout << "oldFilename=" << oldFilename << std::endl;

    auto dirFixed = FixPath(dir);
    auto filenameFixed = FixPath(filename);
    auto oldFilenameFixed = FixPath(oldFilename);

    while (true) {
      auto pos1 = filenameFixed.find('/');
      auto pos2 = oldFilenameFixed.find('/');
      if (pos1 != std::string::npos && pos1 == pos2 &&
          !memcmp(filenameFixed.data(), oldFilenameFixed.data(), pos1)) {
        dirFixed += '/';
        dirFixed += { filenameFixed.begin(), filenameFixed.begin() + pos1 };
        std::cout << "WOW: " << dirFixed << std::endl;
        filenameFixed = { filenameFixed.begin() + pos1 + 1,
                          filenameFixed.end() };
        oldFilenameFixed = { oldFilenameFixed.begin() + pos1 + 1,
                             oldFilenameFixed.end() };

      } else {
        break;
      }
    }
    dirFixed = FixPath(dirFixed);

    auto matchingDirectoryInfo = Find(dirFixed, rootDirectoryInfo.get());

    if (matchingDirectoryInfo) {
      matchingDirectoryInfo->structureReady = false;

      uint64_t directorySizeInBytesDecrease =
        matchingDirectoryInfo->sizeInBytes;
      for (auto it = matchingDirectoryInfo; it != nullptr; it = it->parent) {
        it->sizeInBytes -= directorySizeInBytesDecrease;
      }

      std::cout << "INVALIDATING " << matchingDirectoryInfo->path.u8string()
                << std::endl;
    }

    searchTask = std::make_unique<SearchTask>(std::move(rootDirectoryInfo));
  }

  DirectoryInfo* Find(const std::string& dirFixed,
                      DirectoryInfo* directoryInfo)
  {
    if (FixPath(directoryInfo->path.u8string()) == dirFixed) {
      return directoryInfo;
    }

    for (auto& [k, v] : directoryInfo->subdirectoryByUtf8Name) {
      std::string fixedPath = FixPath(v->path.u8string());
      if (fixedPath == dirFixed) {
        return v.get();
      }
      if (StartsWith(dirFixed, fixedPath)) {
        return Find(dirFixed, v.get());
      }
    }
    return nullptr;
  }

  efsw::FileWatcher fileWatcher;
  std::optional<efsw::WatchID> rootWatchId;
  std::unique_ptr<SearchTask> searchTask;

  std::mutex searchTaskM;
};

MyController::MyController()
  : pImpl(new Impl)
{
  // FileWatcher supports adding listeners after `watch` call
  pImpl->fileWatcher.watch();
}

void MyController::SetRootPath(const char* newRootPathUtf8)
{
  auto newRootPath = std::filesystem::u8path(newRootPathUtf8);

  if (pImpl->rootWatchId.has_value()) {
    // TODO: Fix deadlock in removeWatch
    pImpl->fileWatcher.removeWatch(pImpl->rootWatchId.value());
    pImpl->rootWatchId.reset();
  }

  if (Exists(newRootPath)) {
    {
      std::lock_guard l(pImpl->searchTaskM);
      if (!pImpl->searchTask ||
          pImpl->searchTask->GetRootPath().compare(newRootPath) != 0) {
        pImpl->searchTask = std::make_unique<SearchTask>(newRootPath);
      }
    }
    pImpl->rootWatchId =
      pImpl->fileWatcher.addWatch(newRootPathUtf8, pImpl.get(), true);
  }
}

bool MyController::Exists(const std::filesystem::path& path)
{
  try {
    // std::filesystem::exists may throw for some system files
    return std::filesystem::exists(path);
  } catch (...) {
    return false;
  }
}

Model MyController::MakeSnapshot() const
{
  return pImpl->searchTask ? pImpl->searchTask->MakeSnapshot() : Model();
}