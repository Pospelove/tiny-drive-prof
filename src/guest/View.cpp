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
};

View::View(std::shared_ptr<Controller> controller)
  : pImpl(new Impl)
{
  pImpl->controller = controller;
}

void View::DrawPieChart(const Model& model) noexcept
{
  std::vector<const char*> labels;
  std::vector<std::string> labelsStr;
  std::vector<float> data;
  data.resize(model.entries.size());
  labels.resize(model.entries.size());
  labelsStr.resize(model.entries.size());

  uint64_t sumSizes = 0;
  for (auto& entry : model.entries) {
    sumSizes += entry.sizeInBytes;
  }

  uint64_t sumSizesFromEnd = 0;
  uint64_t maxFromEnd = 0;
  size_t i = model.entries.size() - 1;
  for (; i != static_cast<uint64_t>(-1); --i) {
    sumSizesFromEnd += model.entries[i].sizeInBytes;
    maxFromEnd = std::max(maxFromEnd, model.entries[i].sizeInBytes);
    if (static_cast<double>(sumSizesFromEnd) / sumSizes >= 0.075) {
      sumSizesFromEnd -= model.entries[i].sizeInBytes;
      break;
    }
  }

  for (int i = 0; i < labelsStr.size(); ++i) {
    labelsStr[i] = model.entries[i].relativePathUtf8.data();
    labels[i] = labelsStr[i].data();
    data[i] = (model.entries[i].sizeInBytes / 1024ull / 1024ull);
  }

  bool popped = false;
  while (!data.empty() &&
         (sumSizesFromEnd / 1024ull / 1024ull) > data.back()) {
    data.pop_back();
    labels.pop_back();
    popped = true;
  }

  if (popped) {
    labels.push_back("Other");
    data.push_back(sumSizesFromEnd / 1024ull / 1024ull);
  }

  ImPlot::PushColormap(ImPlotColormap_Pastel);
  ImPlot::SetNextPlotLimits(0, 1, 0, 1, ImGuiCond_Always);

  auto plotSize = ImVec2(ImGui::GetWindowHeight(), -1);

  ImGui::SetCursorPosX((ImGui::GetWindowSize().x - plotSize.x) * 0.5f);
  if (ImPlot::BeginPlot("##Pie1", NULL, NULL, plotSize, ImPlotFlags_Equal)) {
    ImPlot::PlotPieChart(labels.data(), data.data(), labels.size(), 0.5, 0.5,
                         0.4, true, "%.0f MB");
    ImPlot::EndPlot();
  }

  ImPlot::PopColormap();
}

void View::Draw(const Model& model) noexcept
{
  ImVec2 windowSize = { ImGui::GetIO().DisplaySize.y / 1.1f,
                        ImGui::GetIO().DisplaySize.y / 1.1f };
  ImVec2 windowPos;

  ImGui::SetNextWindowPos(
    { (ImGui::GetIO().DisplaySize.x - windowSize.x) / 2,
      (ImGui::GetIO().DisplaySize.y - windowSize.y) / 2 },
    ImGuiCond_Appearing);
  ImGui::SetNextWindowSize(windowSize, ImGuiCond_Appearing);
  ImGui::Begin("View", nullptr,
               ImGuiWindowFlags_NoSavedSettings);

  if (ImGui::InputText("Root path", pImpl->rootPath,
                       std::size(pImpl->rootPath))) {
    pImpl->lastInputUse = std::chrono::system_clock::now();
  }
  if (std::chrono::system_clock::now() - pImpl->lastInputUse > 300ms) {
    pImpl->controller->SetRootPath(pImpl->rootPath);
  }

  std::vector<const char*> labels;
  std::vector<double> positions;
  std::vector<uint64_t> size;

  size_t n = model.entries.size();
  labels.resize(n, "");
  positions.resize(n);
  size.resize(n);

  for (size_t i = 0; i < n; ++i) {
    labels[i] = model.entries[i].relativePathUtf8.data();
    positions[i] = i;
    size[i] = model.entries[i].sizeInBytes;
  }

  DrawPieChart(model);

  ImGui::End();
}