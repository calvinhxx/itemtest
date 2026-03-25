#ifndef RATINGCONTROL_H
#define RATINGCONTROL_H

#include <QWidget>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

namespace view::basicinput {

/**
 * @brief RatingControl - WinUI 3 风格的评分控件
 *
 * 支持 1–N 星评分（默认 5 星），半星精度，占位值，
 * 鼠标悬停预览，只读模式，以及可选的右侧标题文字。
 */
class RatingControl : public QWidget, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(double placeholderValue READ placeholderValue WRITE setPlaceholderValue NOTIFY placeholderValueChanged)
    Q_PROPERTY(QString caption READ caption WRITE setCaption NOTIFY captionChanged)
    Q_PROPERTY(bool isClearEnabled READ isClearEnabled WRITE setIsClearEnabled NOTIFY isClearEnabledChanged)
    Q_PROPERTY(bool isReadOnly READ isReadOnly WRITE setIsReadOnly NOTIFY isReadOnlyChanged)
    Q_PROPERTY(int maxRating READ maxRating WRITE setMaxRating NOTIFY maxRatingChanged)
    Q_PROPERTY(int starSize READ starSize WRITE setStarSize NOTIFY starSizeChanged)
    Q_PROPERTY(QString fontRole READ fontRole WRITE setFontRole NOTIFY fontRoleChanged)
    Q_PROPERTY(QString captionFontRole READ captionFontRole WRITE setCaptionFontRole NOTIFY captionFontRoleChanged)

public:
    explicit RatingControl(QWidget* parent = nullptr);

    void onThemeUpdated() override;

    double value() const { return m_value; }
    void setValue(double value);

    double placeholderValue() const { return m_placeholderValue; }
    void setPlaceholderValue(double value);

    QString caption() const { return m_caption; }
    void setCaption(const QString& caption);

    bool isClearEnabled() const { return m_isClearEnabled; }
    void setIsClearEnabled(bool enabled);

    bool isReadOnly() const { return m_isReadOnly; }
    void setIsReadOnly(bool readOnly);

    int maxRating() const { return m_maxRating; }
    void setMaxRating(int rating);

    int starSize() const { return m_starSize; }
    void setStarSize(int size);

    QString fontRole() const { return m_fontRole; }
    void setFontRole(const QString& role);

    QString captionFontRole() const { return m_captionFontRole; }
    void setCaptionFontRole(const QString& role);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void valueChanged(double value);
    void placeholderValueChanged(double value);
    void captionChanged(const QString& caption);
    void isClearEnabledChanged(bool enabled);
    void isReadOnlyChanged(bool readOnly);
    void maxRatingChanged(int rating);
    void starSizeChanged(int size);
    void fontRoleChanged();
    void captionFontRoleChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void enterEvent(QEnterEvent* event) override;

private:
    double ratingFromPosition(int x) const;
    QRectF starRect(int index) const;
    int starsAreaWidth() const;
    QSize iconCellSize() const;

    double m_value = -1.0;            // -1 = 未设置
    double m_placeholderValue = 0.0;
    QString m_caption;
    bool m_isClearEnabled = true;
    bool m_isReadOnly = false;
    int m_maxRating = 5;
    int m_starSize = 16;
    int m_itemSpacing = 4;
    QString m_fontRole = "Body";
    QString m_captionFontRole = "Caption";

    double m_hoverValue = -1.0;       // 悬停预览值
    bool m_isHovered = false;
    bool m_isPressed = false;
};

} // namespace view::basicinput

#endif // RATINGCONTROL_H
