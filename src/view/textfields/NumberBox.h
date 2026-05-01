#ifndef NUMBERBOX_H
#define NUMBERBOX_H

#include "LineEdit.h"

#include <QSize>

class QFocusEvent;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QPainter;
class QResizeEvent;

namespace view::basicinput { class RepeatButton; }

namespace view::textfields {

class NumberBox : public LineEdit {
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(double minimum READ minimum WRITE setMinimum NOTIFY minimumChanged)
    Q_PROPERTY(double maximum READ maximum WRITE setMaximum NOTIFY maximumChanged)
    Q_PROPERTY(double smallChange READ smallChange WRITE setSmallChange NOTIFY smallChangeChanged)
    Q_PROPERTY(double largeChange READ largeChange WRITE setLargeChange NOTIFY largeChangeChanged)
    Q_PROPERTY(QString header READ header WRITE setHeader NOTIFY headerChanged)
    Q_PROPERTY(bool acceptsExpression READ acceptsExpression WRITE setAcceptsExpression NOTIFY acceptsExpressionChanged)
    Q_PROPERTY(SpinButtonPlacementMode spinButtonPlacementMode READ spinButtonPlacementMode WRITE setSpinButtonPlacementMode NOTIFY spinButtonPlacementModeChanged)
    Q_PROPERTY(QSize spinButtonSize READ spinButtonSize WRITE setSpinButtonSize NOTIFY spinButtonSizeChanged)
    Q_PROPERTY(QSize inlineSpinButtonSize READ inlineSpinButtonSize WRITE setInlineSpinButtonSize NOTIFY inlineSpinButtonSizeChanged)
    Q_PROPERTY(int spinButtonRightMargin READ spinButtonRightMargin WRITE setSpinButtonRightMargin NOTIFY spinButtonRightMarginChanged)
    Q_PROPERTY(int compactSpinButtonReservedWidth READ compactSpinButtonReservedWidth WRITE setCompactSpinButtonReservedWidth NOTIFY compactSpinButtonReservedWidthChanged)
    Q_PROPERTY(int spinButtonSpacing READ spinButtonSpacing WRITE setSpinButtonSpacing NOTIFY spinButtonSpacingChanged)
    Q_PROPERTY(int spinButtonTextGap READ spinButtonTextGap WRITE setSpinButtonTextGap NOTIFY spinButtonTextGapChanged)
    Q_PROPERTY(int spinButtonIconSize READ spinButtonIconSize WRITE setSpinButtonIconSize NOTIFY spinButtonIconSizeChanged)
    Q_PROPERTY(int displayPrecision READ displayPrecision WRITE setDisplayPrecision NOTIFY displayPrecisionChanged)
    Q_PROPERTY(double formatStep READ formatStep WRITE setFormatStep NOTIFY formatStepChanged)

public:
    enum class SpinButtonPlacementMode { Hidden, Compact, Inline };
    Q_ENUM(SpinButtonPlacementMode)

    explicit NumberBox(QWidget* parent = nullptr);

    double value() const { return m_value; }
    void setValue(double value);

    double minimum() const { return m_minimum; }
    void setMinimum(double minimum);

    double maximum() const { return m_maximum; }
    void setMaximum(double maximum);
    void setRange(double minimum, double maximum);

    double smallChange() const { return m_smallChange; }
    void setSmallChange(double change);

    double largeChange() const { return m_largeChange; }
    void setLargeChange(double change);

    QString header() const { return m_header; }
    void setHeader(const QString& header);

    bool acceptsExpression() const { return m_acceptsExpression; }
    void setAcceptsExpression(bool accepts);

    SpinButtonPlacementMode spinButtonPlacementMode() const { return m_spinButtonPlacementMode; }
    void setSpinButtonPlacementMode(SpinButtonPlacementMode mode);

    QSize spinButtonSize() const { return m_spinButtonSize; }
    void setSpinButtonSize(const QSize& size);

    QSize inlineSpinButtonSize() const { return m_inlineSpinButtonSize; }
    void setInlineSpinButtonSize(const QSize& size);

    int spinButtonRightMargin() const { return m_spinButtonRightMargin; }
    void setSpinButtonRightMargin(int margin);

    int compactSpinButtonReservedWidth() const { return m_compactSpinButtonReservedWidth; }
    void setCompactSpinButtonReservedWidth(int width);

    int spinButtonSpacing() const { return m_spinButtonSpacing; }
    void setSpinButtonSpacing(int spacing);

    int spinButtonTextGap() const { return m_spinButtonTextGap; }
    void setSpinButtonTextGap(int gap);

    int spinButtonIconSize() const { return m_spinButtonIconSize; }
    void setSpinButtonIconSize(int size);

    int displayPrecision() const { return m_displayPrecision; }
    void setDisplayPrecision(int precision);

    double formatStep() const { return m_formatStep; }
    void setFormatStep(double step);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void onThemeUpdated() override;

signals:
    void valueChanged(double value);
    void minimumChanged(double minimum);
    void maximumChanged(double maximum);
    void smallChangeChanged(double change);
    void largeChangeChanged(double change);
    void headerChanged();
    void acceptsExpressionChanged(bool accepts);
    void spinButtonPlacementModeChanged(SpinButtonPlacementMode mode);
    void spinButtonSizeChanged(const QSize& size);
    void inlineSpinButtonSizeChanged(const QSize& size);
    void spinButtonRightMarginChanged(int margin);
    void compactSpinButtonReservedWidthChanged(int width);
    void spinButtonSpacingChanged(int spacing);
    void spinButtonTextGapChanged(int gap);
    void spinButtonIconSizeChanged(int size);
    void displayPrecisionChanged(int precision);
    void formatStepChanged(double step);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void enterEvent(FluentEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void changeEvent(QEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QRect inputRect() const;
    int inputTop() const;
    int totalPreferredHeight() const;

    void initializeSpinnerButtons();
    void updateSpinButtonIcons();
    void updateChildGeometry();
    void updateSpinnerState();
    void updateTextMarginsForChrome();
    void updateHeaderTextMargins();
    void commitInput();
    void setInvalidValueFromText();
    void stepBy(double delta);
    double normalizedStepStart() const;
    double normalizeValue(double value) const;
    double applyFormatStep(double value) const;
    QString formatValue(double value) const;
    bool parseInputText(const QString& input, double* result) const;
    bool setValueInternal(double value, bool updateText, bool keepUserTextWhenNaN);
    void paintInputFrame(QPainter& painter);
    void paintHeader(QPainter& painter);
    bool hasSpinnerButtonsVisible() const;
    int inlineSpinnerWidth() const;
    int compactExpandedSpinnerWidth() const;

    QString m_header;
    double m_value;
    double m_minimum;
    double m_maximum;
    double m_smallChange = 1.0;
    double m_largeChange = 10.0;
    bool m_acceptsExpression = false;
    SpinButtonPlacementMode m_spinButtonPlacementMode = SpinButtonPlacementMode::Hidden;
    int m_displayPrecision = -1;
    double m_formatStep = 0.0;

    ::view::basicinput::RepeatButton* m_spinUpButton = nullptr;
    ::view::basicinput::RepeatButton* m_spinDownButton = nullptr;
    bool m_hovered = false;
    bool m_focused = false;
    bool m_pressed = false;
    bool m_spinnerHovered = false;
    bool m_spinnerPressed = false;

    QSize m_spinButtonSize = QSize(24, 14);
    QSize m_inlineSpinButtonSize = QSize(24, 20);
    int m_spinButtonRightMargin = 4;
    int m_compactSpinButtonReservedWidth = 14;
    int m_spinButtonSpacing = 2;
    int m_spinButtonTextGap = 2;
    int m_spinButtonIconSize = 10;

    static constexpr int kInputHeight = 32;
    static constexpr int kHeaderHeight = 20;
    static constexpr int kHeaderGap = 8;
    static constexpr int kMinimumWidth = 124;
};

} // namespace view::textfields

#endif // NUMBERBOX_H
