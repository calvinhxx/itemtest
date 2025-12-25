#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include <QString>

#include "items/ResponsivePushbutton.h"
#include "items/ResponsiveLabel.h"
#include "viewmodel/VM_ResponsivePushbutton.h"
#include "viewmodel/VM_ResponsiveLabel.h"
#include "layouts/AnchorLayout.h"

class ResponsivePushbuttonTest : public ::testing::Test {
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
        window->setWindowTitle("ResponsivePushbutton UT Visual Check");
        layout = new AnchorLayout(window);
        window->setLayout(layout);
    }

    void TearDown() override {
        delete window;
    }

    QWidget* window;
    AnchorLayout* layout;
};

TEST_F(ResponsivePushbuttonTest, BindingUpdate) {
    // 1. Setup Label (作为观察对象)
    VM_ResponsiveLabel* vmLabel = new VM_ResponsiveLabel(window);
    ResponsiveLabel* label = new ResponsiveLabel(window);
    label->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50;");
    label->bind(vmLabel, "text");
    vmLabel->setText("点击按钮 5 次将其禁用");

    AnchorLayout::Anchors labelA;
    labelA.horizontalCenter = true;
    labelA.topTo = window; labelA.topOffset = 100;
    layout->addAnchoredWidget(label, labelA);

    // 2. Setup Button
    VM_ResponsivePushbutton* vmBtn = new VM_ResponsivePushbutton(window);
    ResponsivePushbutton* btn = new ResponsivePushbutton(window);
    btn->setFixedSize(200, 50);
    btn->bindText(vmBtn, "text");
    btn->bindEnabled(vmBtn, "enabled");
    vmBtn->setText("点击我");

    AnchorLayout::Anchors btnA;
    btnA.horizontalCenter = true;
    btnA.topTo = label; btnA.topOffset = 50;
    layout->addAnchoredWidget(btn, btnA);

    // 3. Logic
    QObject::connect(btn, &QPushButton::clicked, [vmLabel, vmBtn]() {
        static int count = 0;
        count++;
        vmLabel->setText(QString("按钮已点击 %1 次").arg(count));
        
        if (count >= 5) {
            vmBtn->setText("已达到上限");
            vmBtn->setEnabled(false);
        }
    });

    window->show();

    // 验证初始状态
    EXPECT_EQ(btn->text(), "点击我");
    EXPECT_TRUE(btn->isEnabled());

    qApp->exec();
}
