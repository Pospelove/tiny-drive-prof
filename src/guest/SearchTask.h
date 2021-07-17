#pragma once
#include "DirectoryIndexer.h"
#include "DirectoryInfo.h"
#include "Model.h"
#include <filesystem>
#include <memory>

class SearchTask
{
public:
  SearchTask(const std::filesystem::path& rootPath);
  SearchTask(std::unique_ptr<DirectoryInfo> rootDirectoryInfo);
  ~SearchTask();

  const std::filesystem::path& GetRootPath() const;

  Model MakeSnapshot() const;

  void Stop(std::unique_ptr<DirectoryInfo>& out);

private:
  class Impl;
  std::shared_ptr<Impl> pImpl;

  void FinishTasks();

  static void IndexDirectory(
    DirectoryInfo* directoryInfo,
    const std::shared_ptr<DirectoryIndexer::TaskDestination>& taskDestination);

  static void RestartTasks(DirectoryInfo* directoryInfo,
                           const std::shared_ptr<Impl>& pImpl, int depth);
};