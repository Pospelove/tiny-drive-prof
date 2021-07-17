#include "MyController.h"

void MyController::SetRootPath(const char* newRootPathUtf8)
{
  auto newRootPath = std::filesystem::u8path(newRootPathUtf8);

  if (Exists(newRootPath)) {
    if (!searchTask || searchTask->GetRootPath().compare(newRootPath) != 0) {
      searchTask = std::make_unique<SearchTask>(newRootPath);
    }
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
  return searchTask ? searchTask->MakeSnapshot() : Model();
}