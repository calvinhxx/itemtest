#include "Label.h"

namespace view::textfields {

Label::Label(const QString& text, QWidget* parent)
    : QLabel(text, parent) {
    onThemeUpdated();
}

Label::Label(QWidget* parent)
    : Label("", parent) {
}

void Label::setFluentTypography(const QString& styleName) {
    if (m_styleName == styleName) return;
    m_styleName = styleName;
    setFont(themeFont(m_styleName).toQFont());
    emit typographyChanged();
}

void Label::onThemeUpdated() {
    // 1. 使用保存的样式名更新字体 (解决切换主题后字体统一的问题)
    setFont(themeFont(m_styleName).toQFont());

    // 2. 更新颜色
    const auto& c = themeColors();
    QPalette p = palette();
    p.setColor(QPalette::WindowText, c.textPrimary);
    setPalette(p);
}

} // namespace view::textfields
