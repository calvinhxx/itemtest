#include <gtest/gtest.h>
#include <QApplication>
#include <QVBoxLayout>
#include "view/dialogs_flyouts/Dialog.h"
#include "view/basicinput/Button.h"
#include "view/textfields/TextBlock.h"
#include "view/QMLPlus.h"

using namespace view::dialogs_flyouts;
using namespace view::basicinput;
using namespace view::textfields;
using namespace view;

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
        window->setWindowTitle("Dialog Visual Test");
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

    Button* showBtn = new Button("Show Fluent Dialog", window);
    layout->addWidget(showBtn);

    QObject::connect(showBtn, &Button::clicked, [this]() {
        Dialog dialog(window);
        dialog.setFixedSize(360, 240);
        
        // Dialog 内部使用 AnchorLayout
        auto* dialogLayout = new AnchorLayout(&dialog);
        
        TextBlock* title = new TextBlock("Confirm Action", &dialog);
        title->setFont(title->themeFont("Subtitle").toQFont());
        title->anchors()->top = {&dialog, AnchorLayout::Edge::Top, 20};
        title->anchors()->left = {&dialog, AnchorLayout::Edge::Left, 20};
        dialogLayout->addWidget(title);

        TextBlock* content = new TextBlock("Are you sure you want to proceed with this operation?", &dialog);
        content->setWordWrap(true);
        content->anchors()->top = {title, AnchorLayout::Edge::Bottom, 12};
        content->anchors()->left = {&dialog, AnchorLayout::Edge::Left, 20};
        content->anchors()->right = {&dialog, AnchorLayout::Edge::Right, -20};
        dialogLayout->addWidget(content);

        Button* confirmBtn = new Button("Confirm", &dialog);
        confirmBtn->setFixedSize(100, 32);
        confirmBtn->anchors()->bottom = {&dialog, AnchorLayout::Edge::Bottom, -20};
        confirmBtn->anchors()->right = {&dialog, AnchorLayout::Edge::Right, -20};
        dialogLayout->addWidget(confirmBtn);

        Button* cancelBtn = new Button("Cancel", &dialog);
        cancelBtn->setFixedSize(100, 32);
        cancelBtn->anchors()->bottom = {&dialog, AnchorLayout::Edge::Bottom, -20};
        cancelBtn->anchors()->right = {confirmBtn, AnchorLayout::Edge::Left, -10};
        dialogLayout->addWidget(cancelBtn);

        QObject::connect(confirmBtn, &Button::clicked, &dialog, &QDialog::accept);
        QObject::connect(cancelBtn, &Button::clicked, &dialog, &QDialog::reject);

        dialog.exec();
    });

    window->show();
    qApp->exec();
}
