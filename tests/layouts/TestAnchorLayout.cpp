#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QDebug>
#include "layouts/AnchorLayout.h"
#include "utils/DebugOverlay.h"

class AnchorLayoutTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char **argv = nullptr;
        if (!qApp) {
            new QApplication(argc, argv);
        }
        QApplication::setStyle("Fusion");
    }

    void SetUp() override {
        window = new QWidget();
        window->setFixedSize(600, 600);
        window->setWindowTitle("AnchorLayout UT Visual Check");
        layout = new AnchorLayout(window);
        window->setLayout(layout);
    }

    void TearDown() override {
        delete window;
    }

    QWidget* window;
    AnchorLayout* layout;
};

TEST_F(AnchorLayoutTest, FullScenarioVisualCheck) {
    using Edge = AnchorLayout::Edge;

    // 1) 左上：锚定到父控件 (10, 10)
    QPushButton* btn1 = new QPushButton("左上", window);
    btn1->setFixedSize(100, 100);
    AnchorLayout::Anchors a1;
    a1.left = {window, Edge::Left, 10};
    a1.top  = {window, Edge::Top, 10};
    layout->addAnchoredWidget(btn1, a1);
    new DebugOverlay(btn1, Qt::red, window);

    // 2) btn2 放在 btn1 右侧 (+20)
    QPushButton* btn2 = new QPushButton("右侧 +20", window);
    btn2->setFixedSize(100, 100);
    AnchorLayout::Anchors a2;
    a2.left = {btn1, Edge::Right, 20}; 
    a2.top  = {window, Edge::Top, 10};
    layout->addAnchoredWidget(btn2, a2);
    new DebugOverlay(btn2, Qt::blue, window);

    // 3) 全居中 (250, 250)
    QPushButton* btn3 = new QPushButton("全居中", window);
    btn3->setFixedSize(100, 100);
    AnchorLayout::Anchors a3;
    a3.horizontalCenter = {window, Edge::HCenter, 0};
    a3.verticalCenter   = {window, Edge::VCenter, 0};
    layout->addAnchoredWidget(btn3, a3);
    new DebugOverlay(btn3, Qt::magenta, window);

    // 4) 仅水平居中 (位于底部 -20)
    QPushButton* btn6 = new QPushButton("仅水平居中", window);
    btn6->setFixedSize(120, 40);
    AnchorLayout::Anchors a6;
    a6.bottom = {window, Edge::Bottom, -20};
    a6.horizontalCenter = {window, Edge::HCenter, 0};
    layout->addAnchoredWidget(btn6, a6);
    new DebugOverlay(btn6, Qt::cyan, window);

    // 5) 填充区域 (Margins: 0, 200, 400, 200)
    QPushButton* btn4 = new QPushButton("填充区域", window);
    AnchorLayout::Anchors a4;
    a4.fill = true;
    a4.fillMargins = QMargins(0, 200, 400, 200); 
    layout->addAnchoredWidget(btn4, a4);
    new DebugOverlay(btn4, Qt::darkYellow, window);

    // 6) 右下锚定 (-16, -16)
    QPushButton* btn5 = new QPushButton("右下", window);
    btn5->setFixedSize(100, 100);
    AnchorLayout::Anchors a5;
    a5.right  = {window, Edge::Right, -16};
    a5.bottom = {window, Edge::Bottom, -16};
    layout->addAnchoredWidget(btn5, a5);
    new DebugOverlay(btn5, Qt::black, window);

    // 7) 动态 Resize 测试：QLabel 内容在 3 秒后改变
    QLabel* labelDynamic = new QLabel("等待 3 秒后文字会变长...", window);
    labelDynamic->setStyleSheet("background-color: #3498db; color: white; padding: 5px;");
    AnchorLayout::Anchors aDyn;
    aDyn.left = {window, Edge::Left, 20};
    aDyn.top  = {btn1, Edge::Bottom, 20};
    layout->addAnchoredWidget(labelDynamic, aDyn);
    new DebugOverlay(labelDynamic, Qt::red, window);

    QTimer::singleShot(3000, [labelDynamic, this]() {
        labelDynamic->setText("成功！文字变长了，布局和 DebugLine 应该自动跟随。");
    });

    window->show();

    int W = window->width();
    int H = window->height();
    
    // btn1: (10, 10)
    EXPECT_EQ(btn1->pos(), QPoint(10, 10));
    
    // btn2: x = 10 + 100 + 20 = 130
    EXPECT_EQ(btn2->x(), 130);
    
    // btn3: 全居中 (W-100)/2, (H-100)/2
    EXPECT_EQ(btn3->pos(), QPoint((W - 100) / 2, (H - 100) / 2));
    
    // btn6: 仅水平居中，底部偏移 -20
    EXPECT_EQ(btn6->pos(), QPoint((W - 120) / 2, H - 40 - 20));
    
    // btn4: 填充区域 Margins(0, 200, 400, 200)
    EXPECT_EQ(btn4->geometry(), QRect(0, 200, W - 400, H - 400));
    
    // btn5: 右下锚定 (-16, -16)
    EXPECT_EQ(btn5->pos(), QPoint(W - 100 - 16, H - 100 - 16));

    qApp->exec();
}
