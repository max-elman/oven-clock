#include <GfxRenderer.h>
#include <Logging.h>
#include <Theme.h>

#include <cstdlib>
#include <ctime>

#include "../ui/Elements.h"
#include "ClockApp.h"

namespace papyrix::clock_app {

namespace {
// Days since 1970-01-01 for a civil date (Howard Hinnant's algorithm).
int64_t daysFromCivil(int y, unsigned m, unsigned d) {
  y -= m <= 2;
  const int64_t era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = static_cast<unsigned>(y - era * 400);
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + static_cast<int64_t>(doe) - 719468;
}
// Day of month of the n-th Sunday (n >= 1) of a month.
int nthSunday(int year, unsigned month, int n) {
  const int64_t first = daysFromCivil(year, month, 1);
  const int weekday = static_cast<int>(((first % 7) + 11) % 7);  // 0 = Sunday (1970-01-01 was a Thursday)
  return 1 + (7 - weekday) % 7 + 7 * (n - 1);
}
}  // namespace

// UTC seconds -> US Pacific local seconds. DST runs from 2:00 PST on the second Sunday of March
// (10:00 UTC) to 2:00 PDT on the first Sunday of November (09:00 UTC).
int64_t pacificLocalSeconds(int64_t utc) {
  const std::time_t standard = static_cast<std::time_t>(utc - 8 * 3600);
  std::tm t{};
  gmtime_r(&standard, &t);
  const int year = t.tm_year + 1900;
  const int64_t dstStart = daysFromCivil(year, 3, nthSunday(year, 3, 2)) * 86400 + 10 * 3600;
  const int64_t dstEnd = daysFromCivil(year, 11, nthSunday(year, 11, 1)) * 86400 + 9 * 3600;
  return utc + (utc >= dstStart && utc < dstEnd ? -7 : -8) * 3600;
}

void renderFrame(GfxRenderer& renderer, const ClockFrame& frame) {
  const auto& appearance = frame.appearance;
  Theme theme{};
  theme.invertedMode = appearance.invertedMode;
  theme.selectionFillBlack = appearance.selectionFillBlack;
  theme.selectionTextBlack = appearance.selectionTextBlack;
  theme.primaryTextBlack = appearance.primaryTextBlack;
  theme.secondaryTextBlack = appearance.secondaryTextBlack;
  theme.backgroundColor = appearance.backgroundColor;
  theme.screenMarginTop = appearance.screenMarginTop;
  theme.screenMarginSide = appearance.screenMarginSide;
  theme.itemHeight = appearance.itemHeight;
  theme.itemSpacing = appearance.itemSpacing;
  theme.itemPaddingX = appearance.itemPaddingX;
  theme.itemValuePadding = appearance.itemValuePadding;
  theme.uiFontId = UI_FONT_ID;
  theme.smallFontId = SMALL_FONT_ID;
  if (clock_faces::isDark(frame.face)) {
    // Oven clock: white on black for the dark face, whatever the reader's theme is.
    theme.invertedMode = true;
    theme.primaryTextBlack = false;
    theme.secondaryTextBlack = false;
    theme.selectionFillBlack = false;
    theme.selectionTextBlack = true;
    theme.backgroundColor = 0x00;
  }
  renderer.setRenderMode(GfxRenderer::BW);
  renderer.setOrientation(static_cast<GfxRenderer::Orientation>(appearance.orientation));
  renderer.clearScreen(theme.backgroundColor);

  // Oven clock: US Pacific time worked out here, not through the C library's TZ handling
  // (that left the clock on UTC on the device).
  const std::time_t local = static_cast<std::time_t>(pacificLocalSeconds(frame.utcSeconds));
  std::tm timeinfo{};
  const bool valid = gmtime_r(&local, &timeinfo) != nullptr;
  LOG_INF("CLOCK_FRAME", "utc=%lld local=%02d:%02d face=%d", static_cast<long long>(frame.utcSeconds),
          timeinfo.tm_hour, timeinfo.tm_min, static_cast<int>(frame.face));
  if (frame.utcSeconds >= 946684800 && valid) {
    clock_faces::render({renderer, theme, timeinfo, frame.use24h != 0, frame.dateFormat, frame.utcOffset}, frame.face);
  } else {
    renderer.drawCenteredText(theme.uiFontId, (renderer.getScreenHeight() - renderer.getLineHeight(theme.uiFontId)) / 2,
                              "Time not synced", theme.primaryTextBlack);
  }

  constexpr int batteryBodyWidth = 30;
  constexpr int batteryTipWidth = 3;
  constexpr int batteryRightMargin = 20;
  // Oven clock: time only; the battery shows up only when it needs charging.
  if (frame.interactive || (frame.batteryPercent >= 0 && frame.batteryPercent < 15 && !frame.usbConnected)) {
    ui::batteryIcon(renderer, theme,
                    renderer.getScreenWidth() - batteryRightMargin - batteryBodyWidth - batteryTipWidth, 22,
                    frame.batteryPercent, frame.usbConnected);
  }
  if (frame.interactive) {
    ui::setFrontButtonLayout(appearance.frontButtonLayout);
    ui::buttonBar(renderer, theme, ui::ButtonBar("Back", "Menu", "", ""));
  }
}

}  // namespace papyrix::clock_app
