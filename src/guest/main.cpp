#include "App.h"
#include <cr.h>

std::unique_ptr<App> g_app;

CR_EXPORT int cr_main(cr_plugin* ctx, cr_op operation)
{
  if (!g_app) {
    auto hostData = reinterpret_cast<HostData*>(ctx->userdata);
    g_app = std::make_unique<App>(hostData);
  }

  switch (operation) {
    case CR_LOAD:
      g_app->Load();
      break;
    case CR_UNLOAD:
      g_app->Unload();
      break;
    case CR_CLOSE:
      g_app->Close();
      break;
    case CR_STEP:
      g_app->Step();
      break;
  }
  return 0;
}