#include "ClockReport.h"

#include <Arduino.h>
#include <Logging.h>
#include <SdFat.h>
#include <WiFi.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "../core/Core.h"

#define TAG "CLOCK_REPORT"

#ifndef PAPYRIX_VERSION
#define PAPYRIX_VERSION "unknown"
#endif

namespace papyrix::clock_app {
namespace {

constexpr const char* kUrlPath = "/.papyrix/apps/homeassistant.txt";
constexpr uint32_t kTimeoutMs = 3000;

struct Target {
  char host[64] = {};
  uint16_t port = 80;
  char path[160] = {};
};

bool readUrl(Core& core, char* out, size_t capacity) {
  const auto exists = core.storage.exists(kUrlPath);
  if (!exists.ok() || !exists.value) return false;
  FsFile file;
  if (!core.storage.openRead(kUrlPath, file).ok()) return false;
  const int read = file.read(reinterpret_cast<uint8_t*>(out), capacity - 1);
  file.close();
  if (read <= 0) return false;
  out[read] = '\0';
  // First line only, trimmed.
  char* end = out;
  while (*end && *end != '\n' && *end != '\r') ++end;
  *end = '\0';
  while (end > out && end[-1] == ' ') *--end = '\0';
  return *out != '\0';
}

bool parseUrl(const char* url, Target& out) {
  constexpr const char* scheme = "http://";
  if (std::strncmp(url, scheme, std::strlen(scheme)) != 0) return false;
  const char* host = url + std::strlen(scheme);
  const char* hostEnd = host;
  while (*hostEnd && *hostEnd != ':' && *hostEnd != '/') ++hostEnd;
  const size_t hostLength = hostEnd - host;
  if (hostLength == 0 || hostLength >= sizeof(out.host)) return false;
  std::memcpy(out.host, host, hostLength);
  out.host[hostLength] = '\0';
  const char* rest = hostEnd;
  if (*rest == ':') {
    char* portEnd = nullptr;
    const long port = std::strtol(rest + 1, &portEnd, 10);
    if (port <= 0 || port > 65535) return false;
    out.port = static_cast<uint16_t>(port);
    rest = portEnd;
  }
  if (*rest != '/' || std::strlen(rest) >= sizeof(out.path)) return false;
  std::strcpy(out.path, rest);
  return true;
}

}  // namespace

void reportStatus(Core& core) {
  if (!core.wifi.isConnected()) return;
  char url[224];
  if (!readUrl(core, url, sizeof(url))) return;
  Target target;
  if (!parseUrl(url, target)) {
    LOG_ERR(TAG, "Bad Home Assistant URL (http:// only)");
    return;
  }

  const auto battery = core.battery.readStatus();
  char body[192];
  const int bodyLength = std::snprintf(
      body, sizeof(body), "{\"battery\":%d,\"millivolts\":%d,\"charging\":%s,\"usb\":%s,\"rssi\":%d,\"version\":\"%s\"}",
      battery.percentageKnown ? static_cast<int>(battery.percentage) : -1,
      battery.millivoltsKnown ? static_cast<int>(battery.millivolts) : -1,
      battery.chargingKnown && battery.charging ? "true" : "false", core.usb.isConnected() ? "true" : "false",
      static_cast<int>(core.wifi.signalStrength()), PAPYRIX_VERSION);
  if (bodyLength <= 0 || static_cast<size_t>(bodyLength) >= sizeof(body)) return;

  WiFiClient client;
  client.setTimeout(kTimeoutMs);
  if (!client.connect(target.host, target.port, kTimeoutMs)) {
    LOG_ERR(TAG, "Home Assistant not reachable");
    return;
  }
  client.printf(
      "POST %s HTTP/1.1\r\nHost: %s:%u\r\nContent-Type: application/json\r\nContent-Length: %d\r\n"
      "Connection: close\r\n\r\n",
      target.path, target.host, static_cast<unsigned>(target.port), bodyLength);
  client.write(reinterpret_cast<const uint8_t*>(body), bodyLength);

  // Read just the status line, so the request finishes before Wi-Fi goes off.
  const uint32_t start = millis();
  while (!client.available() && client.connected() && millis() - start < kTimeoutMs) delay(10);
  char status[48] = {};
  if (client.available()) client.readBytesUntil('\n', status, sizeof(status) - 1);
  client.stop();
  LOG_INF(TAG, "Reported battery %s: %s", body, status);
}

}  // namespace papyrix::clock_app
