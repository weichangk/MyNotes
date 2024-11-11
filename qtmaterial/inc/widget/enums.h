#include "qtmaterial_global.h"

namespace widget {

static const char *kWidgetStateProperty = "widget-state";

namespace light {
static const char *kIconNormalIconColor = "#9aa0aa";
static const char *kIconHoverIconColor = "#5b6474";
static const char *kIconPressedIconColor = "#5b6474";
static const char *kIconCheckedIconColor = "#5b6474";
static const char *kIconDisabledIconColor = "#f0f3f6";

//
static const char *kIconHasBgNormalBgColor = "#f7eef7";
static const char *kIconHasBgHoverBgColor = "#f5e7f5";
static const char *kIconHasBgPressedBgColor = "#f5e7f5";
static const char *kIconHasBgCheckedBgColor = "#f5e7f5";
static const char *kIconHasBgDisabledBgColor = "#ecedef";

static const char *kIconHasBgNormalIconColor = "#9aa0aa";
static const char *kIconHasBgHoverIconColor = "#9aa0aa";
static const char *kIconHasBgPressedIconColor = "#9aa0aa";
static const char *kIconHasBgCheckedIconColor = "#9aa0aa";
static const char *kIconHasBgDisabledIconColor = "#ffffff";

//
static const char *kIconTextNormalIconColor = "#9aa0aa";
static const char *kIconTextHoverIconColor = "#5b6474";
static const char *kIconTextPressedIconColor = "#5b6474";
static const char *kIconTextCheckedIconColor = "#5b6474";
static const char *kIconTextDisabledIconColor = "#f0f3f6";

static const char *kIconTextNormalTextColor = "#9aa0aa";
static const char *kIconTextHoverTextColor = "#5b6474";
static const char *kIconTextPressedTextColor = "#5b6474";
static const char *kIconTextCheckedTextColor = "#5b6474";
static const char *kIconTextDisabledTextColor = "#f0f3f6";

//
static const char *kIconTextHasBgNormalBgColor = "transparent";
static const char *kIconTextHasBgHoverBgColor = "#e4e8ec";
static const char *kIconTextHasBgPressedBgColor = "#e4e8ec";
static const char *kIconTextHasBgCheckedBgColor = "#fd3d4b";
static const char *kIconTextHasBgDisabledBgColor = "#transparent";

static const char *kIconTextHasBgNormalIconColor = "#9aa0aa";
static const char *kIconTextHasBgHoverIconColor = "#9aa0aa";
static const char *kIconTextHasBgPressedIconColor = "#9aa0aa";
static const char *kIconTextHasBgCheckedIconColor = "#ffffff";
static const char *kIconTextHasBgDisabledIconColor = "#f0f3f6";

static const char *kIconTextHasBgNormalTextColor = "#9aa0aa";
static const char *kIconTextHasBgHoverTextColor = "#9aa0aa";
static const char *kIconTextHasBgPressedTextColor = "#9aa0aa";
static const char *kIconTextHasBgCheckedTextColor = "#ffffff";
static const char *kIconTextHasBgDisabledTextColor = "#f0f3f6";

} // namespace light

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
} // namespace widget