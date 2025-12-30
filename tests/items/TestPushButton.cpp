#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include "items/PushButton.h"
#include "viewmodel/ViewModel.h"
#include "utils/PropertyBinder.h"

class PushButtonTest : public ::testing::Test {
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
        btn = new PushButton(window);
        vm = new ViewModel(window);
    }

    void TearDown() override {
        delete window;
    }

    QWidget* window;
    PushButton* btn;
    ViewModel* vm;
};

TEST_F(PushButtonTest, GenericBinding) {
    // 绑定文字和启用状态
    PropertyBinder::bind(vm, "text", btn, "text");
    PropertyBinder::bind(vm, "enabled", btn, "enabled");

    vm->setText("Click Me");
    EXPECT_EQ(btn->text(), "Click Me");

    vm->setEnabled(false);
    EXPECT_FALSE(btn->isEnabled());
}

