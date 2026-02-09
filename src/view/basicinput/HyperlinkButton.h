#ifndef HYPERLINKBUTTON_H
#define HYPERLINKBUTTON_H

#include "Button.h"
#include <QUrl>

namespace view::basicinput {

/**
 * @brief HyperlinkButton - WinUI 3 风格的超链接按钮
 * 
 * 特点：
 * - 默认文本颜色为 Accent 颜色
 * - 默认无背景，Hover/Pressed 时显示 Subtle 背景
 * - Hover 时文本显示下划线
 * - 支持点击时自动打开 URL
 */
class HyperlinkButton : public Button {
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(bool showUnderline READ showUnderline WRITE setShowUnderline NOTIFY showUnderlineChanged)

public:
    explicit HyperlinkButton(const QString& text = "", QWidget* parent = nullptr);
    explicit HyperlinkButton(const QString& text, const QUrl& url, QWidget* parent = nullptr);

    QUrl url() const { return m_url; }
    void setUrl(const QUrl& url);

    bool showUnderline() const { return m_showUnderline; }
    void setShowUnderline(bool show);

    void onThemeUpdated() override;

signals:
    void urlChanged();
    void showUnderlineChanged();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QUrl m_url;
    bool m_showUnderline = false; // 默认悬停不显示下划线
};

} // namespace view::basicinput

#endif // HYPERLINKBUTTON_H
