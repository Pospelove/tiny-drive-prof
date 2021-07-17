#include "DirectoryIndexer.h"

DirectoryIndexer::DirectoryIndexer(
  DirectoryInfo* directoryInfo_,
  const std::shared_ptr<TaskDestination>& taskDestination_)
  : directoryInfo(directoryInfo_)
  , taskDestination(taskDestination_)
{
  directoryInfo->files.clear();
  directoryInfo->subdirectoryByUtf8Name.clear();
}

void DirectoryIndexer::Run()
{
  if (!taskDestination->IsTerminated()) {
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
  if (!taskDestination->IsTerminated()) {
    directoryInfo->structureReady = true;

    for (auto it = directoryInfo; it != nullptr; it = it->parent) {
      it->sizeInBytes += directorySizeInBytesGrowth;
    }
    for (auto directoryInfo : directoriesToIndex) {
      taskDestination->PushDirectoryIndexingTask(directoryInfo,
                                                 taskDestination);
    }
  }
}