#include <gtest/gtest.h>
#include <QApplication>
#include <QVBoxLayout>
#include "view/dialogs&flyouts/Dialog.h"
#include "view/basicinput/Button.h"
#include "view/textfields/Label.h"
#include "view/FluentElement.h"
#include "layouts/AnchorLayout.h"

using namespace view::dialogs_flyouts;
using namespace view::basicinput;
using namespace view::textfields;

class DialogTest : public ::testing::Test {
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
        window->setFixedSize(500, 400);
        window->setWindowTitle("Dialog + AnchorLayout Explorer");
        layout = new QVBoxLayout(window);
        window->setLayout(layout);
    }

    void TearDown() override {
        delete window;
    }

    QWidget* window;
    QVBoxLayout* layout;
};

TEST_F(DialogTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("QT_QPA_PLATFORM") && qEnvironmentVariable("QT_QPA_PLATFORM") == "offscreen") {
        GTEST_SKIP() << "Skipping visual test in offscreen mode";
    }

    auto* showBtn = new Button("Open Complex Anchor Dialog");
    layout->addWidget(showBtn);

    QObject::connect(showBtn, &Button::clicked, [this]() {
        Dialog dialog(window);
        dialog.setWindowTitle("AnchorLayout in Dialog");
        dialog.setFixedSize(450, 300);
        
        auto* anchorLayout = new AnchorLayout(&dialog);
        using Edge = AnchorLayout::Edge;

        auto* bgwidget = new QWidget;
        bgwidget->setFixedHeight(32);
        bgwidget->setStyleSheet("background-color: rgba(128, 128, 128, 0.1); border-top-left-radius: 4px; border-top-right-radius: 4px;");
        AnchorLayout::Anchors aBg;
        aBg.left  = {&dialog, Edge::Left, 0};
        aBg.right = {&dialog, Edge::Right, 0};
        aBg.top   = {&dialog, Edge::Top, 0};
        anchorLayout->addAnchoredWidget(bgwidget, aBg);

        // 1. 标题 (放在 bgwidget 内)
        auto* titleLabel = new Label("Settings");
        titleLabel->setFont(dialog.themeFont("Caption").toQFont());
        AnchorLayout::Anchors a1;
        a1.left           = {bgwidget, Edge::Left, 10};
        a1.verticalCenter = {bgwidget, Edge::VCenter, 0}; 
        anchorLayout->addAnchoredWidget(titleLabel, a1);

        // 5. 顶层关闭按钮 (放在 bgwidget 最右边)
        auto* miniCloseBtn = new Button("✕");
        miniCloseBtn->setFixedSize(32, 32);
        miniCloseBtn->setStyleSheet("background: transparent; border: none; font-size: 14px; color: gray;");
        AnchorLayout::Anchors aClose;
        aClose.right          = {bgwidget, Edge::Right, 0};
        aClose.verticalCenter = {bgwidget, Edge::VCenter, 0};
        anchorLayout->addAnchoredWidget(miniCloseBtn, aClose);
        QObject::connect(miniCloseBtn, &Button::clicked, &dialog, &QDialog::reject);

        // 2. 内容 (居中)
        auto* descLabel = new Label("This dialog demonstrates AnchorLayout inside a custom Dialog.\nAll sub-widgets respect the shadow margins.");
        descLabel->setAlignment(Qt::AlignCenter);
        AnchorLayout::Anchors a2;
        a2.horizontalCenter = {&dialog, Edge::HCenter, 0};
        a2.verticalCenter   = {&dialog, Edge::VCenter, 0};
        anchorLayout->addAnchoredWidget(descLabel, a2);

        // 3. 取消按钮 (右下)
        auto* cancelBtn = new Button("Cancel");
        AnchorLayout::Anchors a3;
        a3.right  = {&dialog, Edge::Right, -20};
        a3.bottom = {&dialog, Edge::Bottom, -20};
        anchorLayout->addAnchoredWidget(cancelBtn, a3);

        // 4. 确定按钮 (取消按钮左侧)
        auto* okBtn = new Button("Confirm");
        AnchorLayout::Anchors a4;
        a4.right  = {cancelBtn, Edge::Left, -12};
        a4.bottom = {&dialog, Edge::Bottom, -20};
        anchorLayout->addAnchoredWidget(okBtn, a4);
        
        QObject::connect(cancelBtn, &Button::clicked, &dialog, &QDialog::reject);
        QObject::connect(okBtn, &Button::clicked, &dialog, &QDialog::accept);
        
        dialog.exec();
    });

    window->show();
    qApp->exec();
}
