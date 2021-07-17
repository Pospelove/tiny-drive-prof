#include "SearchTask.h"
#include <thread_pool.hpp>

struct SearchTask::Impl
{
  thread_pool pool = 1;
  std::unique_ptr<DirectoryInfo> rootDirectoryInfo;
  std::atomic<bool> terminated = false;
};

SearchTask::SearchTask(const std::filesystem::path& rootPath)
  : pImpl(new Impl)
{
  pImpl->rootDirectoryInfo =
    std::make_unique<DirectoryInfo>(rootPath, nullptr);

  pImpl->pool.push_task(IndexDirectory, pImpl->rootDirectoryInfo.get(), pImpl);
}

SearchTask::~SearchTask()
{
  pImpl->terminated = true;
  pImpl->pool.wait_for_tasks();
}

const std::filesystem::path& SearchTask::GetRootPath() const
{
  return pImpl->rootDirectoryInfo->path;
}

Model SearchTask::MakeSnapshot() const
{
  Model model;
  if (pImpl->rootDirectoryInfo->structureReady) {
    for (auto& [key, value] :
         pImpl->rootDirectoryInfo->subdirectoryByUtf8Name) {
      model.entries.push_back(
        { key, value->sizeInBytes, Model::EntryType::File });
    }
    for (const auto& fileEntry : pImpl->rootDirectoryInfo->files) {
      model.entries.push_back(fileEntry);
    }
  }
  std::sort(model.entries.begin(), model.entries.end(),
            [](const Model::Entry& lhs, const Model::Entry& rhs) {
              return lhs.sizeInBytes > rhs.sizeInBytes;
            });
  return model;
}

class DirectoryIndexer
{
public:
  DirectoryIndexer(DirectoryInfo* directoryInfo_,
                   const std::shared_ptr<SearchTask::Impl>& pImpl_);

  void Run();

private:
  void ProcessFile(const std::filesystem::directory_iterator& it);
  void FinishProcessing();

  DirectoryInfo* const directoryInfo;
  const std::shared_ptr<SearchTask::Impl>& pImpl;

  uint64_t directorySizeInBytesGrowth = 0;
  std::vector<DirectoryInfo*> directoriesToIndex;
};

DirectoryIndexer::DirectoryIndexer(
  DirectoryInfo* directoryInfo_,
  const std::shared_ptr<SearchTask::Impl>& pImpl_)
  : directoryInfo(directoryInfo_)
  , pImpl(pImpl_)
{
  directoryInfo->files.clear();
}

void DirectoryIndexer::Run()
{
  if (!pImpl->terminated) {
    try {
      auto begin = std::filesystem::directory_iterator(directoryInfo->path);
      auto end = std::filesystem::directory_iterator();

      for (auto it = begin; it != end; ++it) {
        ProcessFile(it);
      }
      FinishProcessing();
    } catch (std::filesystem::filesystem_error&) {
      // Do nothing. The directory would be empty in UI
    }
  }
}

void DirectoryIndexer::ProcessFile(
  const std::filesystem::directory_iterator& it)
{
  auto utf8Name = it->path().filename().u8string();

  if (it->is_directory()) {
    auto& [pair, _] = directoryInfo->subdirectoryByUtf8Name.insert(
      { utf8Name,
        std::make_unique<DirectoryInfo>(it->path(), directoryInfo) });
    auto& [key, newDirectoryInfo] = *pair;
    return directoriesToIndex.push_back(newDirectoryInfo.get());
  }

  if (it->is_symlink()) {
    // ...
  }

  Model::Entry entry = { utf8Name, it->file_size(), Model::EntryType::File };
  directorySizeInBytesGrowth += entry.sizeInBytes;
  directoryInfo->files.push_back(entry);
}

void DirectoryIndexer::FinishProcessing()
{
  if (!pImpl->terminated) {
    directoryInfo->structureReady = true;

    for (auto it = directoryInfo; it != nullptr; it = it->parent) {
      it->sizeInBytes += directorySizeInBytesGrowth;
    }
    for (auto directoryInfo : directoriesToIndex) {
      pImpl->pool.push_task(SearchTask::IndexDirectory, directoryInfo, pImpl);
    }
  }
}

void SearchTask::IndexDirectory(DirectoryInfo* directoryInfo,
                                const std::shared_ptr<Impl>& pImpl_)
{
  DirectoryIndexer directoryIndexer(directoryInfo, pImpl_);
  directoryIndexer.Run();
}