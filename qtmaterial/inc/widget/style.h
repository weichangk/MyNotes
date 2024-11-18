#pragma once
#include "qtmaterial_global.h"

namespace style {

static const char *kWidgetStateProperty = "widget-state";

enum WidgetStatus {
    Normal = 0,
    Hover,
    Pressed,
    Checked,
    Disabled
};

static QString widgetStatusToString(WidgetStatus status) {
    switch (status) {
    case Normal: return "normal";
    case Hover: return "hover";
    case Pressed: return "pressed";
    case Checked: return "checked";
    case Disabled: return "disabled";
    default: return "normal";
    }
}
} // namespace style