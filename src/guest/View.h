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
  void FillDrawData(const Model& model) noexcept;
  void HandleOtherSection(const Model& model) noexcept;
  uint64_t CalculateOtherSectionSize(const Model& model) noexcept;

  static uint64_t SumSizes(const Model& model) noexcept;
  static void ResizeAndCenterNextWindow() noexcept;

  struct Impl;
  std::shared_ptr<Impl> pImpl;
};