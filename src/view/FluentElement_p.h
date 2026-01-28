#ifndef FLUENTELEMENT_P_H
#define FLUENTELEMENT_P_H

#include <QObject>
#include <QSet>
#include "FluentElement.h"

/**
 * @brief FluentThemeManager - 内部全局主题管理器
 * 负责维护所有活跃的 FluentElement 实例并分发主题变更通知。
 */
class FluentThemeManager : public QObject {
    Q_OBJECT
public:
    static FluentThemeManager* instance() {
        static FluentThemeManager inst;
        return &inst;
    }
    
    FluentElement::Theme currentTheme = FluentElement::Light;
    QSet<FluentElement*> elements;

    void notifyAll() {
        // 使用副本遍历，防止回调中对象销毁导致迭代器失效
        auto copy = elements;
        for (auto* e : copy) {
            if (elements.contains(e)) {
                e->onThemeUpdated();
            }
        }
    }
};

/**
 * @brief FluentElementPrivate - FluentElement 的私有数据实现
 */
class FluentElementPrivate {
public:
    explicit FluentElementPrivate(FluentElement* q) : q_ptr(q) {}
    FluentElement* q_ptr;
};

#endif // FLUENTELEMENT_P_H
