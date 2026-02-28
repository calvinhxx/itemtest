#include <gtest/gtest.h>
#include <QApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QFontDatabase>
#include "view/basicinput/ColorPicker.h"
#include "view/basicinput/Button.h"
#include "view/FluentElement.h"

using namespace view;
using namespace view::basicinput;

class ColorPickerTestWindow : public QWidget, public FluentElement {
public:
    using QWidget::QWidget;

    void onThemeUpdated() override {
        const auto& c = themeColors();
        setStyleSheet(QString("background-color: %1;").arg(c.bgCanvas.name()));
    }
};

class ColorPickerTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char** argv = nullptr;
        if (!qApp) new QApplication(argc, argv);
        QApplication::setStyle("Fusion");

        Q_INIT_RESOURCE(resources);
        QFontDatabase::addApplicationFont(":/res/Segoe Fluent Icons.ttf");
        QFontDatabase::addApplicationFont(":/res/SegoeUI-VF.ttf");
    }

    void SetUp() override {
        window = new ColorPickerTestWindow();
        window->setFixedSize(520, 620);
        window->setWindowTitle("Fluent ColorPicker Visual Test");

        auto* root = new QVBoxLayout(window);
        root->setContentsMargins(24, 24, 24, 24);
        root->setSpacing(16);

        auto* title = new QLabel("ColorPicker", window);
        title->setFont(window->themeFont("Title").toQFont());
        root->addWidget(title);

        auto* picker = new ColorPicker(window);
        picker->setAlphaEnabled(true);  // UT 默认开启 Alpha 通道
        root->addWidget(picker, 1);

        auto* status = new QLabel("Color: #FFFFFFFF", window);
        status->setFont(window->themeFont("Body").toQFont());
        root->addWidget(status);

        QObject::connect(picker, &ColorPicker::colorChanged, [status](const QColor& c) {
            status->setText(QString("Color: #%1%2%3%4")
                                .arg(c.red(), 2, 16, QLatin1Char('0'))
                                .arg(c.green(), 2, 16, QLatin1Char('0'))
                                .arg(c.blue(), 2, 16, QLatin1Char('0'))
                                .arg(c.alpha(), 2, 16, QLatin1Char('0')).toUpper());
        });

        auto* themeBtn = new Button("Switch Theme", window);
        themeBtn->setFixedSize(120, 32);
        root->addWidget(themeBtn, 0, Qt::AlignLeft);
        QObject::connect(themeBtn, &Button::clicked, []() {
            FluentElement::setTheme(FluentElement::currentTheme() == FluentElement::Light
                                        ? FluentElement::Dark
                                        : FluentElement::Light);
        });

        window->onThemeUpdated();
    }

    void TearDown() override {
        delete window;
    }

    ColorPickerTestWindow* window = nullptr;
};

TEST_F(ColorPickerTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("QT_QPA_PLATFORM") &&
        qEnvironmentVariable("QT_QPA_PLATFORM") == "offscreen") {
        GTEST_SKIP() << "Skipping visual test in offscreen mode";
    }

    window->show();
    qApp->exec();
}

