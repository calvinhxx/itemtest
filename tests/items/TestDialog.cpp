#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

#include "items/Dialog.h"
#include "items/Label.h"
#include "items/PushButton.h"
#include "viewmodel/ViewModel.h"
#include "layouts/AnchorLayout.h"
#include "utils/PropertyBinder.h"

class DialogTest : public ::testing::Test {
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
        window->setWindowTitle("Dialog Meta Binding Test");
        layout = new AnchorLayout(window);
        window->setLayout(layout);
    }

    void TearDown() override {
        delete window;
    }

    QWidget* window;
    AnchorLayout* layout;
};

TEST_F(DialogTest, DecoupledDialogVisualCheck) {
    // 1. 创建 Dialog 及其控制 VM
    ViewModel* vmDialog = new ViewModel(window);
    Dialog* dialog = new Dialog(window);
    dialog->setMinimumSize(300, 200);
    
    // 使用通用绑定器 (支持双向绑定以同步关闭状态)
    PropertyBinder::bind(vmDialog, "title", dialog, "windowTitle");
    PropertyBinder::bind(vmDialog, "visible", dialog, "visible", PropertyBinder::TwoWay);

    // 2. 外部注入内容
    QVBoxLayout* dialogLayout = new QVBoxLayout(dialog);
    
    ViewModel* vmContent = new ViewModel(dialog);
    Label* contentLabel = new Label(dialog);
    contentLabel->setAlignment(Qt::AlignCenter);
    contentLabel->setStyleSheet("font-size: 18px; color: #d35400;");
    
    // 使用通用绑定器
    PropertyBinder::bind(vmContent, "text", contentLabel, "text");
    vmContent->setText("我是被注入的响应式内容");

    dialogLayout->addWidget(contentLabel);

    // 3. 主界面触发按钮
    ViewModel* vmBtn = new ViewModel(window);
    PushButton* btn = new PushButton(window);
    btn->setFixedSize(200, 50);
    
    // 使用通用绑定器
    PropertyBinder::bind(vmBtn, "text", btn, "text");
    vmBtn->setText("弹出自定义对话框");

    AnchorLayout::Anchors btnA;
    btnA.horizontalCenter = true;
    btnA.verticalCenter = true;
    layout->addAnchoredWidget(btn, btnA);

    // 4. 交互逻辑：仅修改 VM
    QObject::connect(btn, &QPushButton::clicked, [vmDialog, vmContent]() {
        static int count = 0;
        count++;
        vmDialog->setTitle(QString("Meta Binding 演示 %1").arg(count));
        vmContent->setText(QString("通过通用 Binder 更新: %1").arg(count));
        vmDialog->setVisible(true);
    });

    window->show();
    EXPECT_FALSE(dialog->isVisible());

    // qApp->exec(); 
}

