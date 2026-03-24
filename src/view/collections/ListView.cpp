#include "ListView.h"

#include <algorithm>

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QItemSelection>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QShowEvent>
#include <QTimer>
#include <QVariantAnimation>

#include "common/Animation.h"
#include "common/Typography.h"
#include "view/scrolling/ScrollBar.h"

namespace view::collections {

ListView::ListView(QWidget* parent)
    : QListView(parent) {

    m_fontRole = Typography::FontRole::Body;

    setFrameStyle(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMouseTracking(true);

    QListView::setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(this, &QAbstractItemView::clicked, this, [this](const QModelIndex& idx) {
        emit itemClicked(idx.row());
    });

    m_vScrollBar = new ::view::scrolling::ScrollBar(Qt::Vertical, this);
    m_vScrollBar->setObjectName(QStringLiteral("fluentListViewScrollBar"));
    m_vScrollBar->hide();

    auto* innerVBar = verticalScrollBar();
    m_vScrollBar->setRange(innerVBar->minimum(), innerVBar->maximum());
    m_vScrollBar->setPageStep(innerVBar->pageStep());
    m_vScrollBar->setValue(innerVBar->value());

    connect(innerVBar, &QScrollBar::rangeChanged,
            this, [this, innerVBar](int, int) {
                if (!m_vScrollBar) return;
                m_vScrollBar->setRange(innerVBar->minimum(), innerVBar->maximum());
                m_vScrollBar->setPageStep(innerVBar->pageStep());
                suppressNativeScrollBars();
                layoutScrollBar();
            });
    connect(innerVBar, &QScrollBar::valueChanged,
            this, [this](int v) {
                if (m_vScrollBar && m_vScrollBar->value() != v)
                    m_vScrollBar->setValue(v);
            });
    connect(m_vScrollBar, &QScrollBar::valueChanged,
            this, [this, innerVBar](int v) {
                if (innerVBar->value() != v)
                    innerVBar->setValue(v);
            });

    m_accentAnim = new QVariantAnimation(this);
    m_accentAnim->setDuration(::Animation::Duration::Normal);
    m_accentAnim->setEasingCurve(
        ::Animation::getEasing(::Animation::EasingType::Decelerate));
    connect(m_accentAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant& v) {
        m_selectionAccentProgress = v.toReal();
        emit selectionAccentProgressChanged();
        if (viewport()) viewport()->update();
    });

    suppressNativeScrollBars();
    onThemeUpdated();
}

void ListView::setSelectionMode(ListSelectionMode mode) {
    if (m_selectionMode == mode) return;
    m_selectionMode = mode;

    switch (mode) {
    case ListSelectionMode::None:
        QListView::setSelectionMode(QAbstractItemView::NoSelection);
        break;
    case ListSelectionMode::Single:
        QListView::setSelectionMode(QAbstractItemView::SingleSelection);
        break;
    case ListSelectionMode::Multiple:
        QListView::setSelectionMode(QAbstractItemView::MultiSelection);
        break;
    case ListSelectionMode::Extended:
        QListView::setSelectionMode(QAbstractItemView::ExtendedSelection);
        break;
    }
    emit selectionModeChanged();
}

void ListView::setFontRole(const QString& role) {
    if (m_fontRole == role) return;
    m_fontRole = role;
    applyThemeStyle();
    emit fontRoleChanged();
}

int ListView::selectedIndex() const {
    auto idxList = selectionModel()->selectedIndexes();
    return idxList.isEmpty() ? -1 : idxList.first().row();
}

QList<int> ListView::selectedRows() const {
    QSet<int> seen;
    for (const auto& idx : selectionModel()->selectedIndexes())
        seen.insert(idx.row());
    QList<int> rows(seen.begin(), seen.end());
    std::sort(rows.begin(), rows.end());
    return rows;
}

void ListView::setSelectedIndex(int index) {
    const QAbstractItemModel* m = model();
    if (!m || index < 0 || index >= m->rowCount()) {
        clearSelection();
        return;
    }
    setCurrentIndex(m->index(index, 0));
}

::view::scrolling::ScrollBar* ListView::verticalFluentScrollBar() const {
    return m_vScrollBar;
}

void ListView::restartSelectionAccentAnimation() {
    if (!m_accentAnim)
        return;
    m_accentAnim->stop();
    m_selectionAccentProgress = 0.0;
    emit selectionAccentProgressChanged();
    if (viewport()) viewport()->update();
    m_accentAnim->setStartValue(0.0);
    m_accentAnim->setEndValue(1.0);
    m_accentAnim->start();
}

void ListView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    QListView::selectionChanged(selected, deselected);
    if (selected.isEmpty())
        return;
    restartSelectionAccentAnimation();
}

void ListView::paintEvent(QPaintEvent* event) {
    QPainter p(viewport());
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(viewport()->rect(), themeColors().bgLayer);
    p.end();

    QListView::paintEvent(event);
}

void ListView::resizeEvent(QResizeEvent* event) {
    QListView::resizeEvent(event);
    suppressNativeScrollBars();
    layoutScrollBar();
}

void ListView::showEvent(QShowEvent* event) {
    QListView::showEvent(event);
    suppressNativeScrollBars();
    layoutScrollBar();
    // QComboBox 弹层 / macOS 等会在布局后再把内置条拉起来，下一帧再压一次
    QTimer::singleShot(0, this, [this]() {
        suppressNativeScrollBars();
        layoutScrollBar();
    });
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void ListView::enterEvent(QEnterEvent* event) {
    setViewportHovered(true);
    QListView::enterEvent(event);
}
#else
void ListView::enterEvent(QEvent* event) {
    setViewportHovered(true);
    QListView::enterEvent(event);
}
#endif

void ListView::leaveEvent(QEvent* event) {
    setViewportHovered(false);
    QListView::leaveEvent(event);
}

void ListView::setViewportHovered(bool hovered) {
    if (m_viewportHovered == hovered)
        return;
    m_viewportHovered = hovered;
    emit viewportHoveredChanged();
}

void ListView::onThemeUpdated() {
    applyThemeStyle();
}

void ListView::applyThemeStyle() {
    const auto& c = themeColors();

    QPalette pal = palette();
    pal.setColor(QPalette::Base, Qt::transparent);
    pal.setColor(QPalette::Window, Qt::transparent);
    pal.setColor(QPalette::Text, c.textPrimary);
    pal.setColor(QPalette::Highlight, Qt::transparent);
    pal.setColor(QPalette::HighlightedText, c.textPrimary);
    setPalette(pal);

    setFont(themeFont(m_fontRole).toQFont());

    if (viewport()) {
        viewport()->setAutoFillBackground(false);
        QPalette vpal = viewport()->palette();
        vpal.setColor(QPalette::Base, Qt::transparent);
        vpal.setColor(QPalette::Window, Qt::transparent);
        viewport()->setPalette(vpal);
    }

    update();
}

void ListView::layoutScrollBar() {
    if (!m_vScrollBar) return;
    QRect r = rect();
    int x = r.right() - m_vScrollBar->thickness() + 1;
    int y = r.top() + 2;
    int h = r.height() - 4;
    m_vScrollBar->setGeometry(x, y, m_vScrollBar->thickness(), h);
    if (m_vScrollBar->isVisible())
        m_vScrollBar->raise();
}

void ListView::suppressNativeScrollBars() {
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    if (QScrollBar* vsb = verticalScrollBar()) {
        vsb->setAttribute(Qt::WA_DontShowOnScreen, true);
        vsb->hide();
    }
    if (QScrollBar* hsb = horizontalScrollBar()) {
        hsb->setAttribute(Qt::WA_DontShowOnScreen, true);
        hsb->hide();
    }

    if (m_vScrollBar) {
        const bool need = verticalScrollBar()->maximum() > verticalScrollBar()->minimum();
        m_vScrollBar->setVisible(need);
        if (need)
            m_vScrollBar->raise();
    }
}

void ListView::refreshFluentScrollChrome() {
    suppressNativeScrollBars();
    layoutScrollBar();
}

} // namespace view::collections
