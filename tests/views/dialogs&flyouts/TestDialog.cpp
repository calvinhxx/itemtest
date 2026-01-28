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
        
        // 1. 标题 (左上)
        auto* titleLabel = new Label("Settings");
        titleLabel->setFont(dialog.themeFont("Subtitle").toQFont());
        AnchorLayout::Anchors a1;
        a1.leftTo = &dialog; a1.leftOffset = 20;
        a1.topTo = &dialog;  a1.topOffset = 20;
        anchorLayout->addAnchoredWidget(titleLabel, a1);

        // 2. 内容 (居中)
        auto* descLabel = new Label("This dialog demonstrates AnchorLayout inside a custom Dialog.\nAll sub-widgets respect the shadow margins.");
        descLabel->setAlignment(Qt::AlignCenter);
        AnchorLayout::Anchors a2;
        a2.horizontalCenter = true;
        a2.verticalCenter = true;
        anchorLayout->addAnchoredWidget(descLabel, a2);

        // 3. 取消按钮 (右下)
        auto* cancelBtn = new Button("Cancel");
        AnchorLayout::Anchors a3;
        a3.rightTo = &dialog; a3.rightOffset = -20;
        a3.bottomTo = &dialog; a3.bottomOffset = -20;
        anchorLayout->addAnchoredWidget(cancelBtn, a3);

        // 4. 确定按钮 (取消按钮左侧)
        auto* okBtn = new Button("Confirm");
        AnchorLayout::Anchors a4;
        a4.rightTo = cancelBtn; a4.rightOffset = -12;
        a4.bottomTo = &dialog;  a4.bottomOffset = -20;
        anchorLayout->addAnchoredWidget(okBtn, a4);
        
        QObject::connect(cancelBtn, &Button::clicked, &dialog, &QDialog::reject);
        QObject::connect(okBtn, &Button::clicked, &dialog, &QDialog::accept);
        
        dialog.exec();
    });

    window->show();
    qApp->exec();
}
