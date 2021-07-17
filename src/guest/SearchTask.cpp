#include "SearchTask.h"
#include "DirectoryIndexer.h"
#include <thread_pool.hpp>

class SearchTask::Impl : public DirectoryIndexer::TaskDestination
{
public:
  void PushDirectoryIndexingTask(
    DirectoryInfo* directoryInfo,
    std::shared_ptr<TaskDestination> taskDestination) override
  {
    pool.push_task(IndexDirectory, directoryInfo, taskDestination);
  }

  bool IsTerminated() const override { return terminated; }

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

void SearchTask::IndexDirectory(
  DirectoryInfo* directoryInfo,
  const std::shared_ptr<DirectoryIndexer::TaskDestination>& taskDestination)
{
  DirectoryIndexer directoryIndexer(directoryInfo, taskDestination);
  directoryIndexer.Run();
}