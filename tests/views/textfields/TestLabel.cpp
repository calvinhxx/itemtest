#include <gtest/gtest.h>
#include <QApplication>
#include <QVBoxLayout>
#include <QPushButton>
#include "view/textfields/Label.h"

using namespace view::textfields;

class LabelTest : public ::testing::Test {
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
        window->setFixedSize(300, 200);
        window->setWindowTitle("Label Visual Test");
        layout = new QVBoxLayout(window);
        window->setLayout(layout);
    }

    void TearDown() override {
        delete window;
    }

    QWidget* window;
    QVBoxLayout* layout;
};

TEST_F(LabelTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("QT_QPA_PLATFORM") && qEnvironmentVariable("QT_QPA_PLATFORM") == "offscreen") {
        GTEST_SKIP() << "Skipping visual test in offscreen mode";
    }

    Label* label = new Label("Fluent Design Label");
    layout->addWidget(label);
    
    QPushButton* toggleThemeBtn = new QPushButton("Toggle Theme");
    layout->addWidget(toggleThemeBtn);
    
    QObject::connect(toggleThemeBtn, &QPushButton::clicked, []() {
        FluentElement::setTheme(FluentElement::currentTheme() == FluentElement::Light ? FluentElement::Dark : FluentElement::Light);
    });

    window->show();
    qApp->exec();
}
