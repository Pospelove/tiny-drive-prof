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
  ~SearchTask();

  const std::filesystem::path& GetRootPath() const;

  Model MakeSnapshot() const;

private:
  class Impl;
  std::shared_ptr<Impl> pImpl;

  static void IndexDirectory(
    DirectoryInfo* directoryInfo,
    const std::shared_ptr<DirectoryIndexer::TaskDestination>& taskDestination);
};