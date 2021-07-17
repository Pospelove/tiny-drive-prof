#pragma once
#include <imgui.h>
#include <implot.h>

struct HostData
{
  ImGuiContext* imguiContext = nullptr;
  ImPlotContext* implotContext = nullptr;
  void* (*memAlloc)(size_t, void*) = nullptr;
  void (*memFree)(void*, void*) = nullptr;
};