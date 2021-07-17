#pragma once
#include <filesystem>

class Controller
{
public:
  virtual ~Controller() = default;
  virtual void SetRootPath(const char* newRootPathUtf8) = 0;
};