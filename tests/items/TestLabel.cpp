#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include <QFontDatabase>
#include <QFont>
#include <QVBoxLayout>
#include <QTimer>
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

        // 初始化静态库中的资源
        Q_INIT_RESOURCE(resources);

        // 通过 QRC 加载 Fluent 字体资源
        int iconFontId = QFontDatabase::addApplicationFont(":/res/Segoe Fluent Icons.ttf");
        int uiFontId = QFontDatabase::addApplicationFont(":/res/SegoeUI-VF.ttf");
        
        ASSERT_NE(iconFontId, -1) << "Failed to load Segoe Fluent Icons.ttf from resources";
        ASSERT_NE(uiFontId, -1) << "Failed to load SegoeUI-VF.ttf from resources";
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
    PropertyBinder::bind(vm, "text", label, "text");
    EXPECT_EQ(label->text(), "");
    vm->setText("Hello Meta Binding");
    EXPECT_EQ(label->text(), "Hello Meta Binding");
}

TEST_F(LabelTest, UseFluentIconFont) {
    QFont iconFont("Segoe Fluent Icons");
    label->setFont(iconFont);
    
    QString iconChar = QChar(0xE700);
    label->setText(iconChar);
    
    EXPECT_EQ(label->text(), iconChar);
    EXPECT_EQ(label->font().family(), "Segoe Fluent Icons");
}

TEST_F(LabelTest, VisualExample) {
    window->setWindowTitle("Fluent UI Visual Example");
    window->resize(400, 300);
    window->setStyleSheet("background-color: #f3f3f3;");

    QVBoxLayout* layout = new QVBoxLayout(window);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(10);

    // 1. 图标展示
    Label* iconLabel = new Label(window);
    QFont iconFont("Segoe Fluent Icons", 24);
    iconLabel->setFont(iconFont);
    iconLabel->setText(QChar(0xE700)); // Hamburger
    iconLabel->setStyleSheet("color: #0078d4;");
    layout->addWidget(iconLabel);

    // 2. 文本展示
    Label* textLabel = new Label("This is Segoe UI Variable Text", window);
    // 匹配变量字体名称
    QString targetFamily = "Segoe UI Variable Text";
    if (!QFontDatabase::families().contains(targetFamily)) {
        for (const QString& f : QFontDatabase::families()) {
            if (f.contains("Segoe UI Variable", Qt::CaseInsensitive)) {
                targetFamily = f;
                break;
            }
        }
    }
    textLabel->setFont(QFont(targetFamily, 14));
    textLabel->setStyleSheet("color: #333333;");
    layout->addWidget(textLabel);

    EXPECT_NE(iconLabel->text(), "");

    window->show();

    // qApp->exec();
}

