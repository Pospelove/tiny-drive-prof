#include "App.h"
#include "MyController.h"
#include "View.h"
#include <imgui.h>
#include <implot.h>

struct App::Impl
{
  const HostData* const hostData;
  std::shared_ptr<MyController> controller;
  std::shared_ptr<View> view;
};

App::App(const HostData* hostData)
  : pImpl(new Impl{ hostData })
{
}

void App::Load()
{
  ImGui::SetCurrentContext(pImpl->hostData->imguiContext);
  ImGui::SetAllocatorFunctions(pImpl->hostData->memAlloc,
                               pImpl->hostData->memFree);
  ImPlot::SetCurrentContext(pImpl->hostData->implotContext);

  pImpl->controller = std::make_shared<MyController>();
  pImpl->view = std::make_shared<View>(pImpl->controller);
}

void App::Unload()
{
}

void App::Close()
{
  pImpl->view.reset();
}

void App::Step()
{
  if (pImpl->view) {
    pImpl->view->Draw(pImpl->controller->MakeSnapshot());
  }
}