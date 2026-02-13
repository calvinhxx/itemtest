#include "ToolTip.h"
#include "view/textfields/TextBlock.h"
#include "common/Typography.h"
#include "common/Spacing.h"
#include <QVBoxLayout>
#include <QPainter>

namespace view::status_info {

using namespace view::textfields;

ToolTip::ToolTip(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    
    m_textBlock = new TextBlock(this);
    // Ensure label background is transparent so ToolTip's paintEvent handles the background
    m_textBlock->setAttribute(Qt::WA_TranslucentBackground);
    m_textBlock->setStyleSheet("background-color: transparent;");
    
    // ToolTip text style: Default to Caption size, non-bold
    QFont f = m_textBlock->font();
    f.setBold(false); 
    f.setPixelSize(::Typography::FontSize::Caption);
    m_textBlock->setFont(f);
    m_textBlock->setAlignment(Qt::AlignCenter);

    // Initialize margins using Spacing constants
    m_margins = QMargins(::Spacing::Small, ::Spacing::XSmall, 
                        ::Spacing::Small, ::Spacing::XSmall);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(m_margins);
    layout->addWidget(m_textBlock);
    
    setLayout(layout);
    
    // Initial color setup
    const auto& c = themeColors();
    m_bgColor = c.bgSolid; 
    m_borderColor = c.strokeDivider;
    m_textColor = c.textPrimary;
}

void ToolTip::setText(const QString& text) {
    m_textBlock->setText(text);
    adjustSize();
}

QString ToolTip::text() const {
    return m_textBlock->text();
}

void ToolTip::setMargins(const QMargins& margins) {
    if (m_margins != margins) {
        m_margins = margins;
        if (layout()) {
            layout()->setContentsMargins(m_margins);
        }
        adjustSize();
        emit marginsChanged();
    }
}

void ToolTip::setFont(const QFont& font) {
    QWidget::setFont(font);
    if (m_textBlock) {
        m_textBlock->setFont(font);
    }
    adjustSize();
}

void ToolTip::onThemeUpdated() {
    const auto& c = themeColors();
    m_bgColor = c.bgSolid; 
    m_borderColor = c.strokeDivider;
    m_textColor = c.textPrimary;
    update();
}

void ToolTip::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto& r = themeRadius();
    
    // Draw background
    p.setBrush(m_bgColor);
    p.setPen(QPen(m_borderColor, 1));
    p.drawRoundedRect(rect().adjusted(0,0,-1,-1), r.overlay, r.overlay);
}

} // namespace view::status_info
