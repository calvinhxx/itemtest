#ifndef PROGRESSRING_H
#define PROGRESSRING_H

#include <QBasicTimer>
#include <QColor>
#include <QWidget>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

namespace view::status_info {

class ProgressRing : public QWidget, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged)
    Q_PROPERTY(bool isIndeterminate READ isIndeterminate WRITE setIsIndeterminate NOTIFY isIndeterminateChanged)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum NOTIFY minimumChanged)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum NOTIFY maximumChanged)
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(ProgressRingSize ringSize READ ringSize WRITE setRingSize NOTIFY ringSizeChanged)
    Q_PROPERTY(qreal strokeWidth READ strokeWidth WRITE setStrokeWidth NOTIFY strokeWidthChanged)
    Q_PROPERTY(ProgressRingStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(bool backgroundVisible READ backgroundVisible WRITE setBackgroundVisible NOTIFY backgroundVisibleChanged)

public:
    enum class ProgressRingSize { Small, Medium, Large };
    Q_ENUM(ProgressRingSize)

    enum class ProgressRingStatus { Running, Paused, Error };
    Q_ENUM(ProgressRingStatus)

    explicit ProgressRing(QWidget* parent = nullptr);
    ~ProgressRing() override;

    bool isActive() const { return m_isActive; }
    void setIsActive(bool active);

    bool isIndeterminate() const { return m_isIndeterminate; }
    void setIsIndeterminate(bool indeterminate);

    int minimum() const { return m_minimum; }
    void setMinimum(int minimum);

    int maximum() const { return m_maximum; }
    void setMaximum(int maximum);

    void setRange(int minimum, int maximum);

    int value() const { return m_value; }
    void setValue(int value);

    ProgressRingSize ringSize() const { return m_ringSize; }
    void setRingSize(ProgressRingSize size);

    qreal strokeWidth() const { return m_strokeWidth; }
    void setStrokeWidth(qreal width);

    ProgressRingStatus status() const { return m_status; }
    void setStatus(ProgressRingStatus status);

    bool backgroundVisible() const { return m_backgroundVisible; }
    void setBackgroundVisible(bool visible);

    double progressRatio() const;
    bool isAnimationRunning() const { return m_animationTimer.isActive(); }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void onThemeUpdated() override;

signals:
    void isActiveChanged(bool active);
    void isIndeterminateChanged(bool indeterminate);
    void minimumChanged(int minimum);
    void maximumChanged(int maximum);
    void valueChanged(int value);
    void ringSizeChanged(ProgressRingSize size);
    void strokeWidthChanged(qreal width);
    void statusChanged(ProgressRingStatus status);
    void backgroundVisibleChanged(bool visible);

protected:
    void paintEvent(QPaintEvent* event) override;
    void timerEvent(QTimerEvent* event) override;
    void changeEvent(QEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    int diameterForSize() const;
    QRectF ringRect(qreal effectiveStrokeWidth) const;
    QColor indicatorColor() const;
    void updateThemeColors();
    void updateAnimationState();
    bool shouldAnimate() const;

    bool m_isActive = false;
    bool m_isIndeterminate = true;
    int m_minimum = 0;
    int m_maximum = 100;
    int m_value = 0;
    ProgressRingSize m_ringSize = ProgressRingSize::Medium;
    qreal m_strokeWidth = 3.0;
    ProgressRingStatus m_status = ProgressRingStatus::Running;
    bool m_backgroundVisible = false;

    QBasicTimer m_animationTimer;
    qreal m_animationPhase = 0.0;

    QColor m_runningColor;
    QColor m_pausedColor;
    QColor m_errorColor;
    QColor m_disabledColor;
    QColor m_trackColor;
};

} // namespace view::status_info

#endif // PROGRESSRING_H