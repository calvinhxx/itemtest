#ifndef TOGGLESWITCH_H
#define TOGGLESWITCH_H

#include <QWidget>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

class QPropertyAnimation;

namespace view::basicinput {

/**
 * @brief ToggleSwitch - WinUI 3 风格的开关控件
 *
 * 支持 On/Off 切换、自定义 Header / OnContent / OffContent 文字、
 * 滑块动画、悬停放大效果、按压拉伸效果，完全遵循 Fluent Design Token。
 */
class ToggleSwitch : public QWidget, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    Q_PROPERTY(bool isOn READ isOn WRITE setIsOn NOTIFY toggled)
    Q_PROPERTY(QString header READ header WRITE setHeader NOTIFY headerChanged)
    Q_PROPERTY(QString onContent READ onContent WRITE setOnContent NOTIFY onContentChanged)
    Q_PROPERTY(QString offContent READ offContent WRITE setOffContent NOTIFY offContentChanged)
    Q_PROPERTY(QString fontRole READ fontRole WRITE setFontRole NOTIFY fontRoleChanged)
    Q_PROPERTY(qreal knobPosition READ knobPosition WRITE setKnobPosition)

public:
    explicit ToggleSwitch(QWidget* parent = nullptr);

    void onThemeUpdated() override;

    bool isOn() const { return m_isOn; }
    void setIsOn(bool on);

    QString header() const { return m_header; }
    void setHeader(const QString& header);

    QString onContent() const { return m_onContent; }
    void setOnContent(const QString& content);

    QString offContent() const { return m_offContent; }
    void setOffContent(const QString& content);

    QString fontRole() const { return m_fontRole; }
    void setFontRole(const QString& role);

    qreal knobPosition() const { return m_knobPosition; }
    void setKnobPosition(qreal pos);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void toggled(bool isOn);
    void headerChanged(const QString& header);
    void onContentChanged(const QString& content);
    void offContentChanged(const QString& content);
    void fontRoleChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void toggle();
    void animateKnob(bool toOn);
    QRectF trackRect() const;
    QRectF knobRect() const;
    int trackWidth() const { return 40; }
    int trackHeight() const { return 20; }
    int contentAreaX() const;

    bool m_isOn = false;
    QString m_header;
    QString m_onContent = "On";
    QString m_offContent = "Off";
    QString m_fontRole = "Body";

    qreal m_knobPosition = 0.0;  // 0.0 = Off, 1.0 = On
    bool m_isHovered = false;
    bool m_isPressed = false;

    QPropertyAnimation* m_knobAnimation = nullptr;
};

} // namespace view::basicinput

#endif // TOGGLESWITCH_H
