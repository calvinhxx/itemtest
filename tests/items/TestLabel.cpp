#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include "items/Label.h"
#include "viewmodel/ViewModel.h"
#include "utils/PropertyBinder.h"

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
        label = new Label(window);
        vm = new ViewModel(window);
    }

    void TearDown() override {
        delete window;
    }

    QWidget* window;
    Label* label;
    ViewModel* vm;
};

TEST_F(LabelTest, GenericBindingUpdate) {
    // 使用通用绑定器
    PropertyBinder::bind(vm, "text", label, "text");

    EXPECT_EQ(label->text(), "");
    vm->setText("Hello Meta Binding");
    EXPECT_EQ(label->text(), "Hello Meta Binding");
}

