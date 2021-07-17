#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

// Theoretically there could be a set of virtual methods for getting actual
// values. But I believe in practice it is not useful for this task. So
// just hardcoding everything as member variables.

struct Model
{
  enum class EntryType
  {
    File,
    Directory
  };

  struct Entry
  {
    // View requires UTF-8 string. Using std::string instead of
    // std::filesystem::path to prevent conversions each frame.
    std::string relativePathUtf8;

    // Just a number to show in UI. The actual implementation leaves zero value
    // and then after some time calculates the actual size of the filesystem
    // object
    uint64_t sizeInBytes = 0;

    // No polymorphism but still better than "bool isDirectory". There are only
    // files/folders. Theoretically, there could be symlinks, sockets, etc
    EntryType entryType = EntryType::File;
  };

  std::vector<Entry> entries;
};