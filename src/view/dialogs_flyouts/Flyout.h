#ifndef FLYOUT_H
#define FLYOUT_H

#include <QPointer>
#include "view/dialogs_flyouts/Popup.h"

namespace view::dialogs_flyouts {

/**
 * @brief Flyout — Fluent Design / WinUI 3 风格的轻量浮层
 *
 * 派生自 Popup，新增「锚点 + Placement」语义，对标 WinUI `Microsoft.UI.Xaml.Controls.Flyout`：
 *  - 通过 showAt(anchor) 附着在某个控件上
 *  - 通过 Placement 控制相对位置 (Top / Bottom / Left / Right / Full / Auto)
 *  - 与锚点之间留 8px 间隙
 *  - 默认 light-dismiss：非 modal、不变暗、点击外部 / Esc 自动关闭
 *  - 自动避免越出 top-level 窗口边界（可选）
 *
 * 与基类 Popup 的差异：
 *  - Popup.setPosition() 是「自由坐标」语义；Flyout 是「锚点 + 方向」语义
 *  - Popup 默认无 anchor；Flyout 必须有 anchor 才有意义
 *  - Flyout 重写 computePosition() 在 open() 流程中根据 anchor 几何 + 自身尺寸实时计算
 */
class Flyout : public Popup {
    Q_OBJECT
    Q_PROPERTY(Placement placement READ placement WRITE setPlacement NOTIFY placementChanged)
    Q_PROPERTY(int anchorOffset READ anchorOffset WRITE setAnchorOffset)
    Q_PROPERTY(bool clampToWindow READ clampToWindow WRITE setClampToWindow)

public:
    enum Placement {
        Top,        ///< 在 anchor 上方
        Bottom,     ///< 在 anchor 下方
        Left,       ///< 在 anchor 左侧
        Right,      ///< 在 anchor 右侧
        Full,       ///< 顶层窗口居中（与基类默认一致）
        Auto,       ///< 优先 Bottom，空间不足时自动反转到 Top
    };
    Q_ENUM(Placement)

    explicit Flyout(QWidget* parent = nullptr);
    ~Flyout() override;

    Placement placement() const          { return m_placement; }
    void      setPlacement(Placement p);

    int       anchorOffset() const       { return m_anchorOffset; }
    void      setAnchorOffset(int px)    { m_anchorOffset = px; }

    bool      clampToWindow() const      { return m_clampToWindow; }
    void      setClampToWindow(bool e)   { m_clampToWindow = e; }

    QWidget*  anchor() const             { return m_anchor.data(); }

    /// 在指定 anchor 控件上弹出 flyout。等价于 setAnchor(anchor) + open()。
    void showAt(QWidget* anchor);

    /// 仅设置 anchor，不立即弹出。
    void setAnchor(QWidget* anchor);

signals:
    void placementChanged(Placement p);

protected:
    /// 重写：根据 anchor 几何 + Placement + 自身尺寸返回 move() 目标坐标
    /// （含 shadow margin 偏移补偿）
    QPoint computePosition() const override;

private:
    /// Auto 模式下根据可用空间决定实际 placement
    Placement resolveAutoPlacement() const;

    /// 把 anchor 的几何映射到 top-level 坐标系
    QRect anchorRectInTopLevel() const;

    /// 在 top-level 矩形内 clamp 卡片位置（基于「卡片左上角」）
    QPoint clampCardPos(const QPoint& cardTopLeft) const;

    QPointer<QWidget> m_anchor;
    Placement m_placement = Bottom;
    int       m_anchorOffset = 8;     // WinUI 标准间距
    bool      m_clampToWindow = true;
};

} // namespace view::dialogs_flyouts

#endif // FLYOUT_H
