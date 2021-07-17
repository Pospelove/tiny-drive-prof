#include "View.h"
#include <chrono>
#include <imgui.h>
#include <implot.h>
#include <mutex>
#include <sstream>

using namespace std::chrono_literals;

struct View::Impl
{
  std::shared_ptr<Controller> controller;

  std::chrono::time_point<std::chrono::system_clock> lastInputUse;
  char rootPath[512] = { 0 };

  std::vector<const char*> labels;
  std::vector<std::string> labelsStr;
  std::vector<float> data;
};

View::View(std::shared_ptr<Controller> controller)
  : pImpl(new Impl)
{
  pImpl->controller = controller;
}

void View::Draw(const Model& model) noexcept
{
  ResizeAndCenterNextWindow();

  if (ImGui::Begin("View", nullptr, ImGuiWindowFlags_NoSavedSettings)) {
    if (ImGui::InputText("Root path", pImpl->rootPath,
                         std::size(pImpl->rootPath))) {
      pImpl->lastInputUse = std::chrono::system_clock::now();
    }
    if (std::chrono::system_clock::now() - pImpl->lastInputUse > 300ms) {
      pImpl->controller->SetRootPath(pImpl->rootPath);
    }
    DrawPieChart(model);
  }
  ImGui::End();
}

void View::DrawPieChart(const Model& model) noexcept
{
  FillDrawData(model);
  HandleOtherSection(model);

  ImPlot::PushColormap(ImPlotColormap_Pastel);
  ImPlot::SetNextPlotLimits(0, 1, 0, 1, ImGuiCond_Always);

  auto plotSize = ImVec2(ImGui::GetWindowHeight(), -1);

  ImGui::SetCursorPosX((ImGui::GetWindowSize().x - plotSize.x) * 0.5f);
  if (ImPlot::BeginPlot("##Pie1", NULL, NULL, plotSize, ImPlotFlags_Equal)) {
    ImPlot::PlotPieChart(pImpl->labels.data(), pImpl->data.data(),
                         pImpl->labels.size(), 0.5, 0.5, 0.4, true, "%.0f MB");
    ImPlot::EndPlot();
  }

  ImPlot::PopColormap();
}

void View::FillDrawData(const Model& model) noexcept
{
  pImpl->data.resize(model.entries.size());
  pImpl->labels.resize(model.entries.size());
  pImpl->labelsStr.resize(model.entries.size());
  for (int i = 0; i < pImpl->labelsStr.size(); ++i) {
    pImpl->labelsStr[i] = model.entries[i].relativePathUtf8.data();
    pImpl->labels[i] = pImpl->labelsStr[i].data();
    pImpl->data[i] = (model.entries[i].sizeInBytes / 1024ull / 1024ull);
  }
}

void View::HandleOtherSection(const Model& model) noexcept
{
  auto otherSectionSize = CalculateOtherSectionSize(model);

  bool popped = false;
  while (!pImpl->data.empty() &&
         (otherSectionSize / 1024ull / 1024ull) > pImpl->data.back()) {
    pImpl->data.pop_back();
    pImpl->labels.pop_back();
    popped = true;
  }

  if (popped) {
    pImpl->labels.push_back("Other");
    pImpl->data.push_back(otherSectionSize / 1024ull / 1024ull);
  }
}

uint64_t View::CalculateOtherSectionSize(const Model& model) noexcept
{
  const auto sumSizes = SumSizes(model);

  uint64_t otherSectionSize = 0;
  uint64_t maxFromEnd = 0;
  size_t i = model.entries.size() - 1;
  for (; i != static_cast<uint64_t>(-1); --i) {
    otherSectionSize += model.entries[i].sizeInBytes;
    maxFromEnd = std::max(maxFromEnd, model.entries[i].sizeInBytes);
    if (static_cast<double>(otherSectionSize) / sumSizes >= 0.075) {
      otherSectionSize -= model.entries[i].sizeInBytes;
      break;
    }
  }

  return otherSectionSize;
}

uint64_t View::SumSizes(const Model& model) noexcept
{
  uint64_t sumSizes = 0;
  for (auto& entry : model.entries) {
    sumSizes += entry.sizeInBytes;
  }
  return sumSizes;
}

void View::ResizeAndCenterNextWindow() noexcept
{
  ImVec2 windowSize = { ImGui::GetIO().DisplaySize.y / 1.1f,
                        ImGui::GetIO().DisplaySize.y / 1.1f };
  ImGui::SetNextWindowPos(
    { (ImGui::GetIO().DisplaySize.x - windowSize.x) / 2,
      (ImGui::GetIO().DisplaySize.y - windowSize.y) / 2 },
    ImGuiCond_Appearing);
  ImGui::SetNextWindowSize(windowSize, ImGuiCond_Appearing);
}