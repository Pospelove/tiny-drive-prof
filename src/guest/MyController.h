#pragma once
#include "Controller.h"
#include "Model.h"
#include "SearchTask.h"

class MyController : public Controller
{
public:
  void SetRootPath(const char* newRootPathUtf8) override;

  bool Exists(const std::filesystem::path& path);

  Model MakeSnapshot() const;

private:
  std::unique_ptr<SearchTask> searchTask;
};