#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

#include "items/interface/ResponsiveDialog.h"
#include "items/interface/ResponsiveLabel.h"
#include "items/interface/ResponsivePushbutton.h"
#include "viewmodel/VM_ResponsiveDialog.h"
#include "viewmodel/VM_ResponsiveLabel.h"
#include "viewmodel/VM_ResponsivePushbutton.h"
#include "layouts/AnchorLayout.h"

class ResponsiveDialogTest : public ::testing::Test {
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
        window->setWindowTitle("ResponsiveDialog UT Visual Check");
        layout = new AnchorLayout(window);
        window->setLayout(layout);
    }

    void TearDown() override {
        delete window;
    }

    QWidget* window;
    AnchorLayout* layout;
};

TEST_F(ResponsiveDialogTest, DecoupledDialogVisualCheck) {
    // 1. 创建 Dialog 及其控制 VM
    VM_ResponsiveDialog* vmDialog = new VM_ResponsiveDialog(window);
    ResponsiveDialog* dialog = new ResponsiveDialog(window);
    dialog->setMinimumSize(300, 200);
    
    // 绑定基础行为
    dialog->bindTitle(vmDialog, "title");
    dialog->bindVisible(vmDialog, "visible");

    // 2. 外部注入内容 (解耦的关键：Dialog 内部不再感知 QLabel)
    QVBoxLayout* dialogLayout = new QVBoxLayout(dialog);
    
    // 我们甚至可以在对话框内部也使用一个响应式的 Label
    VM_ResponsiveLabel* vmContent = new VM_ResponsiveLabel(dialog);
    ResponsiveLabel* contentLabel = new ResponsiveLabel(dialog);
    contentLabel->setAlignment(Qt::AlignCenter);
    contentLabel->setStyleSheet("font-size: 18px; color: #d35400;");
    contentLabel->bind(vmContent, "text");
    vmContent->setText("我是被注入的响应式内容");

    dialogLayout->addWidget(contentLabel);

    // 3. 主界面触发按钮
    VM_ResponsivePushbutton* vmBtn = new VM_ResponsivePushbutton(window);
    ResponsivePushbutton* btn = new ResponsivePushbutton(window);
    btn->setFixedSize(200, 50);
    btn->bindText(vmBtn, "text");
    vmBtn->setText("弹出自定义对话框");

    AnchorLayout::Anchors btnA;
    btnA.horizontalCenter = true;
    btnA.verticalCenter = true;
    layout->addAnchoredWidget(btn, btnA);

    // 4. 交互逻辑
    QObject::connect(btn, &QPushButton::clicked, [vmDialog, vmContent]() {
        static int count = 0;
        count++;
        vmDialog->setTitle(QString("注入式演示 %1").arg(count));
        vmContent->setText(QString("外部控制的内容更新: %1").arg(count));
        vmDialog->setVisible(true);
    });

    // 对话框关闭同步回 VM
    QObject::connect(dialog, &ResponsiveDialog::visibleChanged, [vmDialog](bool visible) {
        vmDialog->setVisible(visible);
    });

    window->show();

    EXPECT_FALSE(dialog->isVisible());

    qApp->exec();
}
