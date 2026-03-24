#include "ListItemAccentAnimator.h"

#include <QAbstractItemView>
#include <QVariantAnimation>

#include "common/Animation.h"

namespace view::collections {

ListItemAccentAnimator::ListItemAccentAnimator(QAbstractItemView* view, QObject* parent)
    : QObject(parent)
    , m_view(view) {
}

qreal ListItemAccentAnimator::progress(const QModelIndex& index) const {
    auto it = m_animations.find(QPersistentModelIndex(index));
    if (it == m_animations.end())
        return 1.0;
    return it.value()->currentValue().toReal();
}

void ListItemAccentAnimator::animateSelection(const QModelIndex& index) {
    QPersistentModelIndex persistent(index);

    if (m_animations.contains(persistent))
        return;

    auto* anim = new QVariantAnimation(this);
    anim->setDuration(::Animation::Duration::Normal);
    anim->setEasingCurve(::Animation::getEasing(::Animation::EasingType::Decelerate));
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);

    m_animations.insert(persistent, anim);

    connect(anim, &QVariantAnimation::valueChanged, this, [this] {
        if (m_view && m_view->viewport())
            m_view->viewport()->update();
    });

    connect(anim, &QVariantAnimation::finished, this, [this, persistent, anim] {
        m_animations.remove(persistent);
        anim->deleteLater();
    });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void ListItemAccentAnimator::cleanup() {
    for (auto it = m_animations.begin(); it != m_animations.end();) {
        if (!it.key().isValid()) {
            it.value()->stop();
            it.value()->deleteLater();
            it = m_animations.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace view::collections
