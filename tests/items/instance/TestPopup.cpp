#include <gtest/gtest.h>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QWidget>
#include <QVBoxLayout>

#include "items/instance/Popup.h"
#include "viewmodel/VM_ResponsiveDialog.h"

#include <QPushButton>

class PopupTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char **argv = nullptr;
        if (!qApp) {
            new QApplication(argc, argv);
        }
    }

    void SetUp() override {
        // 创建主控面板（作为测试背景）
        harness = new QWidget();
        harness->setFixedSize(600, 500);
        harness->setWindowTitle("Popup Animation Test Harness");
        harness->setStyleSheet("background-color: #f5f5f5;");

        // 创建被测弹窗 (harness 作为父对象)
        popup = new Popup(harness);
        popup->setFixedSize(400, 300);
        popup->setCornerRadius(15);
        popup->setShadowWidth(15);
        popup->setBorderColor(QColor(100, 100, 100, 60));
    }

    void TearDown() override {
        delete harness;
    }

    QWidget* harness;
    Popup* popup;
};

TEST_F(PopupTest, AnimationToggleCheck) {
    VM_ResponsiveDialog* vm = new VM_ResponsiveDialog(popup);
    
    // 执行绑定
    popup->bindTitle(vm, "title");
    popup->bindVisible(vm, "visible");
    vm->setTitle("交互式动画弹窗");

    // 创建控制按钮
    QPushButton* toggleBtn = new QPushButton("点击切换 Popup 状态", harness);
    toggleBtn->setFixedSize(220, 50);
    toggleBtn->setStyleSheet(
        "QPushButton { background-color: #2ecc71; color: white; border-radius: 25px; font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #27ae60; }"
        "QPushButton:pressed { background-color: #1e8449; }"
    );

    // 居中按钮
    toggleBtn->move((harness->width() - toggleBtn->width()) / 2, 
                    (harness->height() - toggleBtn->height()) / 2);

    // 交互逻辑：直接调用标准 API show() 和 close()
    QObject::connect(toggleBtn, &QPushButton::clicked, [this]() {
        if (popup->isVisible()) {
            popup->close();
        } else {
            popup->show();
        }
    });

    harness->show();
    
    // 初始状态
    EXPECT_FALSE(popup->isVisible());
    EXPECT_TRUE(popup->windowFlags() & Qt::FramelessWindowHint);

    qApp->exec(); 
}

