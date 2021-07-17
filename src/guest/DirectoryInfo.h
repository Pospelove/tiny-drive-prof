#pragma once
#include "Model.h"
#include <map>
#include <vector>
#include <filesystem>
#include <memory>

class DirectoryInfo
{
public:
  DirectoryInfo(const std::filesystem::path& path_, DirectoryInfo* parent_)
    : path(path_)
    , parent(parent_)
  {
  }

  // No need to synchronize access (constants)
  const std::filesystem::path path;
  DirectoryInfo* const parent;

  // No need to synchronize access (atomic)
  std::atomic<uint64_t> sizeInBytes = 0;

  // Attention: These variables are filled by tasks from thread pool.
  // `structureReady` would be set to `true`.
  // Before that, you must not access these variables at all
  std::atomic<bool> structureReady;
  std::map<std::string, std::unique_ptr<DirectoryInfo>> subdirectoryByUtf8Name;
  std::vector<Model::Entry> files;
};