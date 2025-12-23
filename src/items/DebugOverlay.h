#pragma once

#include <QWidget>
#include <QPointer>

class DebugOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit DebugOverlay(QWidget *parent = nullptr);

    void setOutlineEnabled(bool enabled) { m_enabled = enabled; update(); }
    void setLabelsEnabled(bool enabled) { m_showLabels = enabled; update(); }
    void setSelectionEnabled(bool enabled) { m_selectionEnabled = enabled; }

    void setSelectedWidget(QWidget *w);
    QWidget *selectedWidget() const { return m_selected.data(); }
    void clearSelection() { setSelectedWidget(nullptr); }

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;
    void paintEvent(QPaintEvent *ev) override;

private:
    bool m_enabled = true;
    bool m_showLabels = true;
    bool m_selectionEnabled = true;
    QPointer<QWidget> m_selected;
};