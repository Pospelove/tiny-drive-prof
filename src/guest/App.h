#pragma once
#include "HostData.h"
#include <memory>

class App
{
public:
  App(const HostData* hostData);

  void Load();
  void Unload();
  void Close();
  void Step();

private:
  struct Impl;
  std::shared_ptr<Impl> pImpl;
};