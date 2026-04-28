#ifndef PASSWORDBOX_H
#define PASSWORDBOX_H

#include "LineEdit.h"

class QFocusEvent;
class QMouseEvent;
class QPaintEvent;
class QPainter;
class QResizeEvent;

namespace view::basicinput { class Button; }

namespace view::textfields {

class PasswordBox : public LineEdit {
    Q_OBJECT
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(QString header READ header WRITE setHeader NOTIFY headerChanged)
    Q_PROPERTY(PasswordRevealMode passwordRevealMode READ passwordRevealMode WRITE setPasswordRevealMode NOTIFY passwordRevealModeChanged)

public:
    enum class PasswordRevealMode {
        Peek,
        Hidden,
        Visible
    };
    Q_ENUM(PasswordRevealMode)

    explicit PasswordBox(QWidget* parent = nullptr);

    QString password() const { return text(); }
    QString header() const { return m_header; }
    PasswordRevealMode passwordRevealMode() const { return m_revealMode; }

    void setHeader(const QString& header);
    void setPasswordRevealMode(PasswordRevealMode mode);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void onThemeUpdated() override;

public slots:
    void setPassword(const QString& password);

signals:
    void passwordChanged(const QString& password);
    void headerChanged();
    void passwordRevealModeChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void enterEvent(FluentEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QRect inputRect() const;
    int inputTop() const;
    int totalPreferredHeight() const;

    void initializeRevealButton();
    void updateRevealButtonGeometry();
    void updateRevealButtonState();
    void updateTextMargins();
    void updateHeaderTextMargins();
    void updateEchoMode();
    void setPeekActive(bool active);
    void paintInputFrame(QPainter& painter);
    void paintHeader(QPainter& painter);
    bool canPeekReveal() const;

    QString m_header;
    PasswordRevealMode m_revealMode = PasswordRevealMode::Peek;
    ::view::basicinput::Button* m_revealButton = nullptr;
    ::view::AnchorLayout* m_buttonLayout = nullptr;
    bool m_peekActive = false;
    bool m_hovered = false;
    bool m_focused = false;
    bool m_pressed = false;

    static constexpr int kInputHeight = 32;
    static constexpr int kHeaderHeight = 20;
    static constexpr int kHeaderGap = 8;
    static constexpr int kButtonWidth = 24;
    static constexpr int kButtonHeight = 24;
    static constexpr int kButtonRightMargin = 4;
    static constexpr int kTextButtonGap = 2;
};

} // namespace view::textfields

#endif // PASSWORDBOX_H