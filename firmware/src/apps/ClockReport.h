#pragma once

namespace papyrix {
struct Core;
namespace clock_app {

// Oven clock: send the battery level to a Home Assistant webhook while Wi-Fi is
// already up for a time sync. The webhook URL is one line in
// /.papyrix/apps/homeassistant.txt on the microSD card (plain http:// only, e.g.
// http://192.168.2.195:8123/api/webhook/oven-clock-abc123). No file, no report.
// Never blocks the clock for long: about 3 seconds at most, and failures are ignored.
void reportStatus(Core& core);

}  // namespace clock_app
}  // namespace papyrix
