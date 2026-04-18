#ifndef TEXTBLOCK_H
#define TEXTBLOCK_H

#include <QLabel>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

namespace view::textfields {

/**
 * @brief Label - WinUI 3 风格文本标签组件（兼容旧名 TextBlock）
 */
class TextBlock : public QLabel, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    /** @brief 绑定的 Fluent 排版样式名称 (如 "Title", "Display", "Caption") */
    Q_PROPERTY(QString fluentTypography READ fluentTypography WRITE setFluentTypography NOTIFY typographyChanged)

public:
    explicit TextBlock(const QString& text, QWidget* parent = nullptr);
    explicit TextBlock(QWidget* parent = nullptr);

    QString fluentTypography() const { return m_styleName; }
    void setFluentTypography(const QString& styleName);

    void onThemeUpdated() override;

signals:
    void typographyChanged();

private:
    QString m_styleName = "Body";
};

// 注意：不要在此命名空间引入 `using Label = TextBlock;`。
// Qt5 / Windows SDK 在某些头文件中已存在 `Label` 符号（如 X11 兼容头、MFC/ATL），
// 会触发 MSVC `C2371: "Label": 重定义`。如需简称请在调用处局部别名。

} // namespace view::textfields

#endif // TEXTBLOCK_H
