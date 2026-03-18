#include <gtest/gtest.h>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFontDatabase>
#include <QLineEdit>

#include "view/basicinput/ComboBox.h"
#include "view/basicinput/Button.h"
#include "view/FluentElement.h"

using namespace view;
using namespace view::basicinput;

class ComboBoxTestWindow : public QWidget, public FluentElement {
public:
    using QWidget::QWidget;

    void onThemeUpdated() override {
        const auto& c = themeColors();
        setStyleSheet(QString("background-color: %1;").arg(c.bgCanvas.name()));
    }
};

class ComboBoxTest : public ::testing::Test {
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
        window = new ComboBoxTestWindow();
        window->setFixedSize(800, 600);
        window->setWindowTitle("Fluent ComboBox Visual Test");

        auto* root = new QVBoxLayout(window);
        root->setContentsMargins(40, 40, 40, 40);
        root->setSpacing(24);

        auto* title = new QLabel("ComboBox", window);
        title->setFont(window->themeFont("Title").toQFont());
        root->addWidget(title);

        // 1. A ComboBox with items defined inline and its width set.
        root->addWidget(new QLabel("A ComboBox with items defined inline and its width set.", window));
        auto* colorsCombo = new ComboBox(window);
        colorsCombo->setPlaceholderText("Pick a color");
        colorsCombo->addItems({"Red", "Green", "Blue", "Yellow", "Purple"});
        colorsCombo->setFixedWidth(260);
        root->addWidget(colorsCombo, 0, Qt::AlignLeft);

        // 2. A ComboBox with its ItemsSource set. (font family selector)
        root->addWidget(new QLabel("A ComboBox with its ItemsSource set.", window));
        auto* fontRow = new QHBoxLayout();
        fontRow->setSpacing(12);

        auto* fontLabel = new QLabel("You can set the font used for this text.", window);
        fontLabel->setFont(window->themeFont("Body").toQFont());

        auto* fontCombo = new ComboBox(window);
        fontCombo->addItems({"Segoe UI", "Courier New", "Times New Roman", "Arial"});
        fontCombo->setCurrentText("Courier New");
        fontCombo->setFixedWidth(260);

        QObject::connect(fontCombo, &QComboBox::currentTextChanged, [fontLabel](const QString& family) {
            QFont f = fontLabel->font();
            f.setFamily(family);
            fontLabel->setFont(f);
        });

        fontRow->addWidget(fontCombo, 0);
        fontRow->addWidget(fontLabel, 1);
        root->addLayout(fontRow);

        // 3. An editable ComboBox. (font size selector)
        root->addWidget(new QLabel("An editable ComboBox.", window));
        auto* sizeRow = new QHBoxLayout();
        sizeRow->setSpacing(12);

        auto* sizeCombo = new ComboBox(window);
        sizeCombo->setEditable(true);
        QStringList sizes = {"8", "9", "10", "11", "12", "14", "16", "18", "20", "24", "28", "36", "48", "72"};
        sizeCombo->addItems(sizes);
        sizeCombo->setCurrentText("10");
        sizeCombo->setFixedWidth(120);

        auto* sizeLabel = new QLabel("You can set the font size used for this text.", window);
        sizeLabel->setFont(window->themeFont("Body").toQFont());

        auto updateSize = [sizeCombo, sizeLabel]() {
            bool ok = false;
            int pt = sizeCombo->currentText().toInt(&ok);
            if (!ok || pt <= 0) return;
            QFont f = sizeLabel->font();
            f.setPointSize(pt);
            sizeLabel->setFont(f);
        };
        QObject::connect(sizeCombo, &QComboBox::currentTextChanged, window, [updateSize]() { updateSize(); });
        if (sizeCombo->lineEdit()) {
            QObject::connect(sizeCombo->lineEdit(), &QLineEdit::editingFinished, window, [updateSize]() { updateSize(); });
        }

        sizeRow->addWidget(sizeCombo, 0);
        sizeRow->addWidget(sizeLabel, 1);
        root->addLayout(sizeRow);

        // 底部主题切换按钮
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

    ComboBoxTestWindow* window = nullptr;
};

TEST_F(ComboBoxTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("SKIP_VISUAL_TEST")) {
        GTEST_SKIP() << "Set SKIP_VISUAL_TEST=1 to skip visual tests";
    }

    window->show();
    qApp->exec();
}

