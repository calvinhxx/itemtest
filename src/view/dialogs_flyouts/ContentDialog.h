#ifndef CONTENTDIALOG_H
#define CONTENTDIALOG_H

#include "Dialog.h"

namespace view::basicinput { class Button; }
namespace view::textfields { class TextBlock; }

namespace view::dialogs_flyouts {

/**
 * @brief ContentDialog — WinUI 3 ContentDialog 派生自 Dialog
 *
 * 在 Dialog 纯 view 基础上，添加 Title / Content / Button 区域。
 * 内部子控件通过 AnchorLayout 定位，方便派生类扩展。
 *
 * 布局结构（两区域）：
 *   ┌─────────────────────────────────┐ ← bgLayer 区域
 *   │  [titleLabel]                   │
 *   │  [contentWidget]                │
 *   ├─────────────────────────────────┤ ← 1px strokeDivider
 *   │  [primary] [secondary] [close]  │ ← bgCanvas 区域 (buttonBar)
 *   └─────────────────────────────────┘
 */
class ContentDialog : public Dialog {
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString primaryButtonText READ primaryButtonText WRITE setPrimaryButtonText)
    Q_PROPERTY(QString secondaryButtonText READ secondaryButtonText WRITE setSecondaryButtonText)
    Q_PROPERTY(QString closeButtonText READ closeButtonText WRITE setCloseButtonText)
    Q_PROPERTY(int defaultButton READ defaultButton WRITE setDefaultButton)

public:
    enum ContentDialogButton { None = 0, Primary, Secondary, Close };
    Q_ENUM(ContentDialogButton)

    static constexpr int ResultNone      = QDialog::Rejected; // 0
    static constexpr int ResultPrimary   = QDialog::Accepted; // 1
    static constexpr int ResultSecondary = 2;

    explicit ContentDialog(QWidget *parent = nullptr);

    // --- Title ---
    QString title() const;
    void setTitle(const QString& text);

    // --- Button text ---
    QString primaryButtonText() const;
    void setPrimaryButtonText(const QString& text);

    QString secondaryButtonText() const;
    void setSecondaryButtonText(const QString& text);

    QString closeButtonText() const;
    void setCloseButtonText(const QString& text);

    // --- DefaultButton ---
    int  defaultButton() const;
    void setDefaultButton(int btn);

    // --- Content ---
    QWidget* content() const;
    void setContent(QWidget* widget);

    void onThemeUpdated() override;

signals:
    void primaryButtonClicked();
    void secondaryButtonClicked();
    void closeButtonClicked();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void setupInternalLayout();
    void updateButtonBar();
    void updateContentAnchors();

    view::textfields::TextBlock* m_titleLabel   = nullptr;
    QWidget*                     m_contentWidget = nullptr;
    QWidget*                     m_buttonBar     = nullptr;

    view::basicinput::Button*    m_primaryBtn    = nullptr;
    view::basicinput::Button*    m_secondaryBtn  = nullptr;
    view::basicinput::Button*    m_closeBtn      = nullptr;

    int m_defaultButton = None;
};

} // namespace view::dialogs_flyouts

#endif // CONTENTDIALOG_H
