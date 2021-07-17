#pragma once
#include "DirectoryInfo.h"
#include "Model.h"
#include <filesystem>
#include <memory>

class SearchTask
{
  friend class DirectoryIndexer;

public:
  SearchTask(const std::filesystem::path& rootPath);
  ~SearchTask();

  const std::filesystem::path& GetRootPath() const;

  Model MakeSnapshot() const;

private:
  struct Impl;
  std::shared_ptr<Impl> pImpl;

  static void IndexDirectory(DirectoryInfo* directoryInfo,
                             const std::shared_ptr<Impl>& pImpl_);
};