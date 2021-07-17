#pragma once
#include "DirectoryInfo.h"
#include <thread_pool.hpp>

class DirectoryIndexer
{
public:
  class TaskDestination
  {
  public:
    ~TaskDestination() = default;

    virtual void PushDirectoryIndexingTask(
      DirectoryInfo* directoryInfo,
      std::shared_ptr<TaskDestination> taskDestination) = 0;

    virtual bool IsTerminated() const = 0;
  };

  DirectoryIndexer(DirectoryInfo* directoryInfo_,
                   const std::shared_ptr<TaskDestination>& taskDestination);

  void Run();

private:
  void ProcessFile(const std::filesystem::directory_iterator& it);
  void FinishProcessing();

  DirectoryInfo* const directoryInfo;
  const std::shared_ptr<TaskDestination>& taskDestination;

  uint64_t directorySizeInBytesGrowth = 0;
  std::vector<DirectoryInfo*> directoriesToIndex;
};