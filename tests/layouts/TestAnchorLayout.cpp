#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include <QPushButton>

#include "layouts/AnchorLayout.h"

class AnchorLayoutTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char **argv = nullptr;
        if (!qApp) {
            new QApplication(argc, argv);
        }
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
    // 1) 左上：锚定到父控件 (10, 10)
    QPushButton* btn1 = new QPushButton("左上", window);
    btn1->setFixedSize(100, 100);
    AnchorLayout::Anchors a1;
    a1.leftTo = window; a1.leftOffset = 10;
    a1.topTo = window;  a1.topOffset = 10;
    layout->addAnchoredWidget(btn1, a1);

    // 2) btn2 放在 btn1 右侧 (+20)
    QPushButton* btn2 = new QPushButton("右侧 +20", window);
    btn2->setFixedSize(100, 100);
    AnchorLayout::Anchors a2;
    a2.leftTo = btn1; a2.leftOffset = 20; 
    a2.topTo = window; a2.topOffset = 10;
    layout->addAnchoredWidget(btn2, a2);

    // 3) 全居中 (250, 250)
    QPushButton* btn3 = new QPushButton("全居中", window);
    btn3->setFixedSize(100, 100);
    AnchorLayout::Anchors a3;
    a3.horizontalCenter = true;
    a3.verticalCenter = true;
    layout->addAnchoredWidget(btn3, a3);

    // 4) 仅水平居中 (位于底部 -20)
    QPushButton* btn6 = new QPushButton("仅水平居中", window);
    btn6->setFixedSize(120, 40);
    AnchorLayout::Anchors a6;
    a6.bottomTo = window; a6.bottomOffset = -20;
    a6.horizontalCenter = true;
    layout->addAnchoredWidget(btn6, a6);

    // 5) 仅垂直居中 (位于右侧 -20)
    QPushButton* btn7 = new QPushButton("仅垂直居中", window);
    btn7->setFixedSize(120, 40);
    AnchorLayout::Anchors a7;
    a7.rightTo = window; a7.rightOffset = -20;
    a7.verticalCenter = true;
    layout->addAnchoredWidget(btn7, a7);

    // 6) 填充区域 (Margins: 0, 200, 400, 200)
    QPushButton* btn4 = new QPushButton("填充区域", window);
    AnchorLayout::Anchors a4;
    a4.fill = true;
    a4.fillMargins = QMargins(0, 200, 400, 200); 
    layout->addAnchoredWidget(btn4, a4);

    // 7) 右下锚定 (-16, -16)
    QPushButton* btn5 = new QPushButton("右下", window);
    btn5->setFixedSize(100, 100);
    AnchorLayout::Anchors a5;
    a5.rightTo = window; a5.rightOffset = -16;
    a5.bottomTo = window; a5.bottomOffset = -16;
    layout->addAnchoredWidget(btn5, a5);

    // 执行布局
    window->show();

    // --- 断言验证 (根据 600x600 窗口计算) ---
    
    // btn1: (10, 10)
    EXPECT_EQ(btn1->pos(), QPoint(10, 10));
    
    // btn2: x = 10 + 100 + 20 = 130
    EXPECT_EQ(btn2->x(), 130);
    
    // btn3: (600-100)/2 = 250
    EXPECT_EQ(btn3->pos(), QPoint(250, 250));
    
    // btn6: x = (600-120)/2 = 240, y = 600-40-20 = 540
    EXPECT_EQ(btn6->pos(), QPoint(240, 540));
    
    // btn7: x = 600-120-20 = 460, y = (600-40)/2 = 280
    EXPECT_EQ(btn7->pos(), QPoint(460, 280));
    
    // btn4: x=0, y=200, w=600-0-400=200, h=600-200-200=200
    EXPECT_EQ(btn4->geometry(), QRect(0, 200, 200, 200));
    
    // btn5: x=600-100-16=484, y=600-100-16=484
    EXPECT_EQ(btn5->pos(), QPoint(484, 484));

    qApp->exec();
}
