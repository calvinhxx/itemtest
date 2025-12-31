#include <gtest/gtest.h>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QWidget>
#include <QVBoxLayout>

#include "items/Popup.h"

#include <QPushButton>

#include "items/Label.h"
#include "items/PushButton.h"
#include "viewmodel/ViewModel.h"
#include "layouts/AnchorLayout.h"
#include "utils/PropertyBinder.h"

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
        // 主控面板
        harness = new QWidget();
        harness->setFixedSize(800, 600);
        harness->setWindowTitle("Popup Animation & Layout Test");
        harness->setStyleSheet("background-color: #f0f2f5;");

        // 创建被测弹窗
        popup = new Popup(harness);
        popup->setFixedSize(450, 350);
    }

    void TearDown() override {
        delete harness;
    }

    QWidget* harness;
    Popup* popup;
};

TEST_F(PopupTest, RichContentCheck) {
    // 1. 设置 Popup 内部布局
    AnchorLayout* layout = new AnchorLayout(popup);
    popup->setLayout(layout);

    // 2. 添加标题 (Label)
    ViewModel* vmTitle = new ViewModel(popup);
    Label* titleLabel = new Label(popup);
    titleLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #2c3e50;");
    PropertyBinder::bind(vmTitle, "text", titleLabel, "text");
    vmTitle->setText("温馨提示");

    AnchorLayout::Anchors titleA;
    titleA.horizontalCenter = true;
    titleA.topTo = popup; titleA.topOffset = 25;
    layout->addAnchoredWidget(titleLabel, titleA);

    // 3. 添加内容文字
    Label* contentLabel = new Label("这是一个使用 AnchorLayout 布局的响应式弹窗。所有内部元素会随着弹窗动画同步缩放。", popup);
    contentLabel->setWordWrap(true);
    contentLabel->setFixedWidth(350);
    contentLabel->setAlignment(Qt::AlignCenter);
    contentLabel->setStyleSheet("font-size: 14px; color: #7f8c8d; line-height: 1.5;");

    AnchorLayout::Anchors contentA;
    contentA.horizontalCenter = true;
    contentA.topTo = titleLabel; contentA.topOffset = 40;
    layout->addAnchoredWidget(contentLabel, contentA);

    // 4. 添加操作按钮 (PushButton)
    ViewModel* vmBtn = new ViewModel(popup);
    PushButton* actionBtn = new PushButton(popup);
    actionBtn->setFixedSize(120, 40);
    actionBtn->setStyleSheet(
        "QPushButton { background-color: #3498db; color: white; border-radius: 20px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2980b9; }"
    );
    PropertyBinder::bind(vmBtn, "text", actionBtn, "text");
    vmBtn->setText("我知道了");

    AnchorLayout::Anchors btnA;
    btnA.horizontalCenter = true;
    btnA.bottomTo = popup; btnA.bottomOffset = -25;
    layout->addAnchoredWidget(actionBtn, btnA);

    // 5. 点击弹窗内部按钮关闭弹窗
    QObject::connect(actionBtn, &QPushButton::clicked, popup, &Popup::close);

    // --- 外部控制逻辑 ---
    
    // 创建控制按钮
    QPushButton* toggleBtn = new QPushButton("点击触发 Popup 动画", harness);
    toggleBtn->setFixedSize(220, 50);
    toggleBtn->setStyleSheet(
        "QPushButton { background-color: #2ecc71; color: white; border-radius: 25px; font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #27ae60; }"
    );
    toggleBtn->move((harness->width() - toggleBtn->width()) / 2, 500);

    QObject::connect(toggleBtn, &QPushButton::clicked, [this]() {
        if (popup->isVisible())
            popup->close();
        else
            popup->show();
    });

    harness->show();
    qApp->exec();
}
