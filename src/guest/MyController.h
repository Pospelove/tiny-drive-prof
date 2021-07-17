#pragma once
#include "Controller.h"
#include "Model.h"
#include "SearchTask.h"
#include <memory>

class MyController : public Controller
{
public:
  MyController();

  void SetRootPath(const char* newRootPathUtf8) override;

  bool Exists(const std::filesystem::path& path);

  Model MakeSnapshot() const;

private:
  class Impl;
  std::shared_ptr<Impl> pImpl;
};