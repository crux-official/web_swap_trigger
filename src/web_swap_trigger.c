// A test-only web swap trigger (the manual-reload seam): after N frames (the
// CRUX_SWAP_AFTER env var), call the hybrid host's public swap entry
// (AppHostHotSwapApp) to hot-swap the app to its new version. The host's own
// choreography loads and verifies the new module (the module loader stages the
// rebuilt file) and asks the new build's Migrate for the outcome. The native
// equivalent is the file watcher + the reloader (a change fact); web has no
// file watcher in Phase 1, so the trigger is an explicit frame count. The app
// name is the CRUX_SWAP_APP env var (default "hello").
#include "extension.h"
#include "app_host_api.h"
#include "kernel_types.h"
#include "logging.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint32_t g_frame = 0;
static uint32_t g_swap_after = 0;
static char g_app_name[64] = "hello";

static bool web_swap_trigger_construct(void) {
  const char *after = getenv("CRUX_SWAP_AFTER");
  if (after != NULL) {
    g_swap_after = (uint32_t) atoi(after);
  }
  const char *app = getenv("CRUX_SWAP_APP");
  if (app != NULL && app[0] != '\0') {
    (void) snprintf(g_app_name, sizeof g_app_name, "%s", app);
  }
  if (g_swap_after != 0) {
    Log("web_swap_trigger: armed to swap '%s' after %u frames", g_app_name, g_swap_after);
  } else {
    Log("web_swap_trigger: disarmed (no CRUX_SWAP_AFTER)");
  }
  return true;
}

static void web_swap_trigger_update(double dt) {
  (void) dt;
  if (g_swap_after == 0 || g_frame >= g_swap_after) {
    return;
  }
  g_frame++;
  if (g_frame == g_swap_after) {
    const bool ok = AppHostHotSwapApp(g_app_name);
    if (ok) {
      Log("web_swap_trigger: the swap of '%s' succeeded (frame %u)", g_app_name, g_frame);
    } else {
      LogError("web_swap_trigger: the swap of '%s' was refused (frame %u)", g_app_name, g_frame);
    }
  }
}

ExtensionInterface g_extension_web_swap_trigger = {
    "web_swap_trigger",
    {1, 0, 0},
    web_swap_trigger_construct,
    NULL,
    web_swap_trigger_update,
    NULL,
    NULL,
    NULL,
};
