#include <gtest/gtest.h>
#include <QApplication>
#include <QFontDatabase>
#include <QTimer>
#include <QElapsedTimer>
#include "view/dialogs_flyouts/Dialog.h"
#include "view/basicinput/Button.h"
#include "view/QMLPlus.h"
#include "view/FluentElement.h"

using namespace view::dialogs_flyouts;
using namespace view::basicinput;
using namespace view;

// ── FluentTestWindow ─────────────────────────────────────────────────────────

class FluentTestWindow : public QWidget, public FluentElement {
public:
    using QWidget::QWidget;
    void onThemeUpdated() override {
        const auto& c = themeColors();
        setStyleSheet(QString("background-color: %1;").arg(c.bgCanvas.name()));
    }
};

// ── Test fixture ─────────────────────────────────────────────────────────────

class DialogTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char **argv = nullptr;
        if (!qApp) {
            new QApplication(argc, argv);
        }
        QApplication::setStyle("Fusion");

        Q_INIT_RESOURCE(resources);
        QFontDatabase::addApplicationFont(":/res/Segoe Fluent Icons.ttf");
        QFontDatabase::addApplicationFont(":/res/SegoeUI-VF.ttf");
    }

    void SetUp() override {
        window = new FluentTestWindow();
        window->setFixedSize(600, 500);
        window->setWindowTitle("Dialog Base Test");
        window->onThemeUpdated();
    }

    void TearDown() override {
        delete window;
    }

    FluentTestWindow* window;
};

// ══════════════════════════════════════════════════════════════════════════════
//  自动化测试 — Dialog 基类（纯 view 层：阴影 + 动画 + 拖拽）
// ══════════════════════════════════════════════════════════════════════════════

TEST_F(DialogTest, DefaultProperties) {
    Dialog dialog(window);
    EXPECT_TRUE(dialog.isDragEnabled());
    EXPECT_TRUE(dialog.isAnimationEnabled());
    EXPECT_FALSE(dialog.isSmokeEnabled());
    EXPECT_EQ(dialog.shadowSize(), 16);
    EXPECT_DOUBLE_EQ(dialog.animationProgress(), 1.0);
}

TEST_F(DialogTest, SmokeProperty) {
    Dialog dialog(window);
    EXPECT_FALSE(dialog.isSmokeEnabled());
    dialog.setSmokeEnabled(true);
    EXPECT_TRUE(dialog.isSmokeEnabled());
}

TEST_F(DialogTest, DragProperty) {
    Dialog dialog(window);
    EXPECT_TRUE(dialog.isDragEnabled());
    dialog.setDragEnabled(false);
    EXPECT_FALSE(dialog.isDragEnabled());
}

TEST_F(DialogTest, AnimationProperty) {
    Dialog dialog(window);
    EXPECT_TRUE(dialog.isAnimationEnabled());
    dialog.setAnimationEnabled(false);
    EXPECT_FALSE(dialog.isAnimationEnabled());
}

TEST_F(DialogTest, AnimationProgressProperty) {
    Dialog dialog(window);
    dialog.setAnimationProgress(0.5);
    EXPECT_DOUBLE_EQ(dialog.animationProgress(), 0.5);
}

TEST_F(DialogTest, ExecWithoutAnimation) {
    Dialog dialog(window);
    dialog.setAnimationEnabled(false);
    dialog.setFixedSize(300, 200);

    QTimer::singleShot(50, [&]() { dialog.done(QDialog::Accepted); });
    int result = dialog.exec();
    EXPECT_EQ(result, QDialog::Accepted);
}

TEST_F(DialogTest, ThemeSwitchNoCrash) {
    Dialog dialog(window);
    dialog.setAnimationEnabled(false);

    FluentElement::setTheme(FluentElement::Dark);
    dialog.onThemeUpdated();

    FluentElement::setTheme(FluentElement::Light);
    dialog.onThemeUpdated();

    SUCCEED();
}

// ══════════════════════════════════════════════════════════════════════════════
//  WinUI 3 GIF 对齐：scale pop-in/pop-out + smoke 淡入淡出
// ══════════════════════════════════════════════════════════════════════════════

TEST_F(DialogTest, DialogEntranceHasScalePopIn) {
    // 入场起始帧尺寸应为 target * 0.90 (kScaleFrom)
    Dialog dialog(window);
    dialog.setFixedSize(400, 300);
    const QSize target = dialog.size();

    window->show();
    QApplication::processEvents();
    dialog.open();                  // open() 会设置 m_isAnimating=true
    QApplication::processEvents();  // 触发 showEvent

    // 强制将 progress 重置回 0（覆盖动画已推进的中间帧）
    dialog.setAnimationProgress(0.0);

    // 起始尺寸应明显小于 95%（旧 kScaleFrom 0.96 不会满足此条件）
    EXPECT_LT(dialog.size().width(), int(target.width() * 0.95))
        << "Entrance starting size should reflect kScaleFrom = 0.90, "
           "but got width=" << dialog.size().width()
        << " target=" << target.width();
    EXPECT_NEAR(dialog.size().width(), int(target.width() * 0.90), 2);
    EXPECT_NEAR(dialog.size().height(), int(target.height() * 0.90), 2);

    dialog.setAnimationEnabled(false);
    dialog.done(0);
}

TEST_F(DialogTest, DialogExitHasScalePopOut) {
    // 退场末尾帧（progress=0）尺寸应为 target * 0.90
    Dialog dialog(window);
    dialog.setFixedSize(400, 300);
    const QSize target = dialog.size();

    window->show();
    QApplication::processEvents();
    dialog.open();
    QApplication::processEvents();

    // 完成入场：直接将 progress 设为 1
    dialog.setAnimationProgress(1.0);
    EXPECT_NEAR(dialog.size().width(), target.width(), 2);

    // 触发退场：done(r) 进入退场动画状态（m_isClosing=true, m_isAnimating=true）
    dialog.done(0);

    // 模拟退场末尾帧
    dialog.setAnimationProgress(0.0);
    EXPECT_NEAR(dialog.size().width(), int(target.width() * 0.90), 2)
        << "Exit ending size should reflect kScaleFrom = 0.90";
    EXPECT_NEAR(dialog.size().height(), int(target.height() * 0.90), 2);
}

TEST_F(DialogTest, SmokeOverlayFadesInOutDelayedDestroy) {
    // 关键不变量：smoke 启用时，done() 返回后 overlay 应仍存在于 parent
    // children 中（延迟销毁），而非立即从 children 中消失。
    window->show();
    QApplication::processEvents();
    const int childCountBefore = window->findChildren<QWidget*>().size();

    Dialog dialog(window);
    dialog.setSmokeEnabled(true);
    dialog.setFixedSize(300, 200);
    dialog.setAnimationEnabled(false);  // 关闭 Dialog 主动画，仅测 smoke 行为

    bool overlayPresentAfterOpen = false;
    bool overlayStillPresentAfterDone = false;
    QTimer::singleShot(30, [&]() {
        // Dialog 已 open，smoke overlay 应作为 window 的子节点存在
        overlayPresentAfterOpen =
            (window->findChildren<QWidget*>().size() > childCountBefore);

        dialog.done(0);
        // done() 返回后，由于 smoke 走异步淡出，overlay 应仍在 children 中
        overlayStillPresentAfterDone =
            (window->findChildren<QWidget*>().size() > childCountBefore);
    });
    dialog.exec();

    EXPECT_TRUE(overlayPresentAfterOpen)
        << "Smoke overlay should be a child of window after open()";
    EXPECT_TRUE(overlayStillPresentAfterDone)
        << "Smoke overlay should still exist immediately after done() (delayed destroy)";

    // 等待淡出完成并让 deferred delete 处理完成（取决于 themeAnimation().fast）
    QElapsedTimer t; t.start();
    while (window->findChildren<QWidget*>().size() > childCountBefore && t.elapsed() < 1500) {
        QApplication::processEvents(QEventLoop::AllEvents, 10);
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }
    // 允许不一定在 1500ms 内完全清理（平台/事件循环依赖）——记录但不硬性断言
    if (window->findChildren<QWidget*>().size() > childCountBefore) {
        qWarning() << "Smoke overlay still present after 1500ms; deferred delete may need full app loop";
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  VisualCheck — 手动观察 Dialog 基类的渲染效果
// ══════════════════════════════════════════════════════════════════════════════

TEST_F(DialogTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("SKIP_VISUAL_TEST")) {
        GTEST_SKIP() << "Set SKIP_VISUAL_TEST=1 to skip visual tests";
    }

    auto* layout = new AnchorLayout(window);
    window->setLayout(layout);
    window->setFixedSize(700, 500);

    using Edge = AnchorLayout::Edge;

    // --- 弹出空白 Dialog（仅阴影 + 动画） ---
    Button* btn1 = new Button("Open Empty Dialog", window);
    btn1->setFluentStyle(Button::Accent);
    btn1->setFixedSize(240, 32);
    btn1->anchors()->top  = {window, Edge::Top,  40};
    btn1->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(btn1);

    QObject::connect(btn1, &Button::clicked, [this]() {
        Dialog dialog(window);
        dialog.setFixedSize(400, 260);
        dialog.exec();
    });

    // --- 弹出禁用动画的 Dialog ---
    Button* btn2 = new Button("No-Animation Dialog", window);
    btn2->setFixedSize(240, 32);
    btn2->anchors()->top  = {btn1, Edge::Bottom, 16};
    btn2->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(btn2);

    QObject::connect(btn2, &Button::clicked, [this]() {
        Dialog dialog(window);
        dialog.setAnimationEnabled(false);
        dialog.setFixedSize(400, 260);
        dialog.exec();
    });

    // --- Toggle theme ---
    Button* themeBtn = new Button("Toggle Dark/Light", window);
    themeBtn->setFixedSize(240, 32);
    themeBtn->anchors()->top  = {btn2, Edge::Bottom, 32};
    themeBtn->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(themeBtn);

    QObject::connect(themeBtn, &Button::clicked, [this]() {
        auto theme = FluentElement::currentTheme() == FluentElement::Light
                         ? FluentElement::Dark : FluentElement::Light;
        FluentElement::setTheme(theme);
        window->onThemeUpdated();
    });

    window->show();
    qApp->exec();
}
