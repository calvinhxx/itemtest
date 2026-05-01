#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QPointer>
#include <QWidget>

#include "view/FluentElement.h"
#include "view/QMLPlus.h"

namespace view::windowing {

class TitleBar : public QWidget, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    Q_PROPERTY(QWidget* contentWidget READ contentWidget WRITE setContentWidget NOTIFY contentWidgetChanged)
    Q_PROPERTY(int systemReservedLeadingWidth READ systemReservedLeadingWidth WRITE setSystemReservedLeadingWidth NOTIFY systemReservedLeadingWidthChanged)
    Q_PROPERTY(int titleBarHeight READ titleBarHeight WRITE setTitleBarHeight NOTIFY titleBarHeightChanged)

public:
    explicit TitleBar(QWidget* parent = nullptr);

    static constexpr int defaultTitleBarHeight() { return kDefaultHeight; }

    QWidget* contentHost() const { return const_cast<TitleBar*>(this); }
    QWidget* contentWidget() const { return m_contentWidget; }
    void setContentWidget(QWidget* widget);

    int systemReservedLeadingWidth() const { return m_systemReservedLeadingWidth; }
    void setSystemReservedLeadingWidth(int width);

    int titleBarHeight() const { return m_titleBarHeight; }
    void setTitleBarHeight(int height);

    QVector<QRect> dragExclusionRects() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void onThemeUpdated() override;

signals:
    void contentWidgetChanged(QWidget* widget);
    void systemReservedLeadingWidthChanged(int width);
    void titleBarHeightChanged(int height);
    void dragStarted(const QPoint& globalPos);
    void dragMoved(const QPoint& globalPos);
    void dragFinished();
    void doubleClicked(const QPoint& globalPos);
    void chromeGeometryChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    static constexpr int kDefaultHeight = 36;

    bool isDragExcludedWidget(const QWidget* widget) const;
    void updateContentWidgetAnchor();

    view::AnchorLayout* m_anchorLayout = nullptr;
    QPointer<QWidget> m_contentWidget;
    int m_systemReservedLeadingWidth = 0;
    int m_titleBarHeight = kDefaultHeight;
    bool m_dragging = false;
};

} // namespace view::windowing

#endif // TITLEBAR_H
