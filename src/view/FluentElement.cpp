#include "FluentElement.h"
#include "FluentElement_p.h"
#include "common/ThemeColors.h"
#include "common/Typography.h"
#include "common/Spacing.h"
#include "common/CornerRadius.h"
#include "common/Elevation.h"
#include "common/Animation.h"
#include "common/Material.h"
#include "common/Breakpoints.h"

// --- FluentElement 生命周期管理 ---

FluentElement::FluentElement() : d_ptr(new FluentElementPrivate(this)) {
    FluentThemeManager::instance()->elements.insert(this);
}

FluentElement::~FluentElement() {
    FluentThemeManager::instance()->elements.remove(this);
    delete d_ptr;
}

// --- 静态全局管理 ---

void FluentElement::setTheme(Theme theme) {
    auto* mgr = FluentThemeManager::instance();
    if (mgr->currentTheme != theme) {
        mgr->currentTheme = theme;
        mgr->notifyAll();
    }
}

FluentElement::Theme FluentElement::currentTheme() {
    return FluentThemeManager::instance()->currentTheme;
}

// --- 数据获取实现 ---

FluentElement::Colors FluentElement::themeColors() const {
    bool isDark = currentTheme() == Dark;
    Colors c;
    
    if (isDark) {
        using namespace ThemeColors::Dark;
        // Fill
        c.accentDefault = Fill::AccentDefault;
        c.accentSecondary = Fill::AccentSecondary;
        c.accentTertiary = Fill::AccentTertiary;
        c.accentDisabled = Fill::AccentDisabled;
        c.controlDefault = Fill::ControlDefault;
        c.controlSecondary = Fill::ControlSecondary;
        c.controlTertiary = Fill::ControlTertiary;
        c.controlDisabled = Fill::ControlDisabled;
        c.controlAltSecondary = Fill::ControlAltSecondary;
        c.controlAltTertiary = Fill::ControlAltTertiary;
        c.subtleTransparent = Fill::SubtleTransparent;
        c.subtleSecondary = Fill::SubtleSecondary;
        c.subtleTertiary = Fill::SubtleTertiary;

        // Stroke
        c.strokeDefault = Stroke::ControlDefault;
        c.strokeSecondary = Stroke::ControlSecondary;
        c.strokeStrong = Stroke::ControlStrong;
        c.strokeCard = Stroke::CardDefault;
        c.strokeDivider = Stroke::DividerDefault;
        c.strokeSurface = Stroke::SurfaceDefault;

        // Text
        c.textPrimary = Text::Primary;
        c.textSecondary = Text::Secondary;
        c.textTertiary = Text::Tertiary;
        c.textDisabled = Text::Disabled;
        c.textOnAccent = Text::OnAccentPrimary;

        // Backgrounds & Neutrals
        c.bgCanvas = BackgroundCanvas;
        c.bgLayer = BackgroundLayer;
        c.bgSolid = BackgroundSolid;
        c.grey10 = Grey10;
        c.grey20 = Grey20;
        c.grey30 = Grey30;
        c.grey40 = Grey40;
        c.grey50 = Grey50;
        c.grey60 = Grey60;
        c.grey90 = Grey90;
        c.grey130 = Grey130;
        c.grey160 = Grey160;
        c.grey190 = Grey190;

        // Charts
        c.charts = QList<QColor>(Charts.begin(), Charts.end());
    } else {
        using namespace ThemeColors::Light;
        // Fill
        c.accentDefault = Fill::AccentDefault;
        c.accentSecondary = Fill::AccentSecondary;
        c.accentTertiary = Fill::AccentTertiary;
        c.accentDisabled = Fill::AccentDisabled;
        c.controlDefault = Fill::ControlDefault;
        c.controlSecondary = Fill::ControlSecondary;
        c.controlTertiary = Fill::ControlTertiary;
        c.controlDisabled = Fill::ControlDisabled;
        c.controlAltSecondary = Fill::ControlAltSecondary;
        c.controlAltTertiary = Fill::ControlAltTertiary;
        c.subtleTransparent = Fill::SubtleTransparent;
        c.subtleSecondary = Fill::SubtleSecondary;
        c.subtleTertiary = Fill::SubtleTertiary;

        // Stroke
        c.strokeDefault = Stroke::ControlDefault;
        c.strokeSecondary = Stroke::ControlSecondary;
        c.strokeStrong = Stroke::ControlStrong;
        c.strokeCard = Stroke::CardDefault;
        c.strokeDivider = Stroke::DividerDefault;
        c.strokeSurface = Stroke::SurfaceDefault;

        // Text
        c.textPrimary = Text::Primary;
        c.textSecondary = Text::Secondary;
        c.textTertiary = Text::Tertiary;
        c.textDisabled = Text::Disabled;
        c.textOnAccent = Text::OnAccentPrimary;

        // Backgrounds & Neutrals
        c.bgCanvas = BackgroundCanvas;
        c.bgLayer = BackgroundLayer;
        c.bgSolid = BackgroundSolid;
        c.grey10 = Grey10;
        c.grey20 = Grey20;
        c.grey30 = Grey30;
        c.grey40 = Grey40;
        c.grey50 = Grey50;
        c.grey60 = Grey60;
        c.grey90 = Grey90;
        c.grey130 = Grey130;
        c.grey160 = Grey160;
        c.grey190 = Grey190;

        // Charts
        c.charts = QList<QColor>(Charts.begin(), Charts.end());
    }
    return c;
}

FluentElement::FontStyle FluentElement::themeFont(const QString& styleName) const {
    Typography::FontStyle style;
    if (styleName == "Caption") style = Typography::Styles::Caption;
    else if (styleName == "BodyStrong") style = Typography::Styles::BodyStrong;
    else if (styleName == "Subtitle") style = Typography::Styles::Subtitle;
    else if (styleName == "Title") style = Typography::Styles::Title;
    else if (styleName == "TitleLarge") style = Typography::Styles::TitleLarge;
    else if (styleName == "Display") style = Typography::Styles::Display;
    else style = Typography::Styles::Body;

    return { style.family, style.size, style.weight };
}

FluentElement::Radius FluentElement::themeRadius() const {
    return {
        ::CornerRadius::None, ::CornerRadius::Small, ::CornerRadius::Medium,
        ::CornerRadius::InPage, ::CornerRadius::TopLevel, ::CornerRadius::Large
    };
}

FluentElement::Spacing FluentElement::themeSpacing() const {
    Spacing s;
    s.padding = { 
        ::Spacing::Padding::ControlHorizontal, ::Spacing::Padding::ControlVertical, 
        ::Spacing::Padding::Card, ::Spacing::Padding::Dialog 
    };
    s.gap = { 
        ::Spacing::Gap::Tight, ::Spacing::Gap::Normal, 
        ::Spacing::Gap::Loose, ::Spacing::Gap::Section 
    };
    s.xSmall = ::Spacing::XSmall;
    s.small = ::Spacing::Small;
    s.medium = ::Spacing::Medium;
    s.standard = ::Spacing::Standard;
    s.large = ::Spacing::Large;
    s.xLarge = ::Spacing::XLarge;
    s.xxLarge = ::Spacing::XXLarge;
    return s;
}

FluentElement::Animation FluentElement::themeAnimation() const {
    using namespace ::Animation;
    return {
        Duration::Fast, Duration::Normal, Duration::Slow, Duration::VerySlow,
        getEasing(EasingType::Standard),
        getEasing(EasingType::Accelerate),
        getEasing(EasingType::Decelerate),
        getEasing(EasingType::Entrance),
        getEasing(EasingType::Exit)
    };
}

QString FluentElement::themeMaterial(const QString& type) const {
    bool isDark = currentTheme() == Dark;
    if (type == "Mica") return Material::MicaParams::getStyleSheet(isDark);
    if (type == "Smoke") return Material::SmokeParams::getStyleSheet(isDark);
    return Material::AcrylicParams::getStyleSheet(isDark);
}

Elevation::ShadowParams FluentElement::themeShadow(Elevation::Level level) const {
    return Elevation::getShadow(level, currentTheme() == Dark);
}

int FluentElement::themeBreakpoint(const QString& size) const {
    if (size == "Small") return Breakpoints::Small;
    if (size == "Large") return Breakpoints::Large;
    return Breakpoints::Medium;
}
