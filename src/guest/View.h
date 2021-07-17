#pragma once
#include "Controller.h"
#include "Model.h"
#include <memory>

class View
{
public:
  View(std::shared_ptr<Controller> controller);

  void Draw(const Model& model) noexcept;

private:
  void DrawPieChart(const Model& model) noexcept;

  struct Impl;
  std::shared_ptr<Impl> pImpl;
};