#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QGridLayout>
#include <QFontDatabase>
#include <QDebug>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"
#include "view/basicinput/Button.h"
#include "view/textfields/Label.h"

using namespace view;
using namespace view::basicinput;
using namespace view::textfields;

// 专门用于 Icon 展示的单元格
class IconCell : public QWidget, public QMLPlus {
public:
    IconCell(const QString& glyph, const QString& name, const QString& hex, QWidget* parent = nullptr) 
        : QWidget(parent) {
        setFixedSize(120, 100);
        
        auto* vLayout = new QVBoxLayout(this);
        vLayout->setSpacing(2);
        vLayout->setContentsMargins(5, 5, 5, 5);

        // 图标标签
        iconLabel = new QLabel(glyph, this);
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setFixedSize(40, 40);
        
        // 使用图标字体
        QFont iconFont("Segoe Fluent Icons");
        if (iconFont.family() != "Segoe Fluent Icons") {
            // 回退尝试
            iconFont.setFamily("Segoe MDL2 Assets");
        }
        iconFont.setPixelSize(24);
        iconLabel->setFont(iconFont);
        
        // 名称标签
        nameLabel = new Label(name, this);
        nameLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setStyleSheet("font-size: 11px; color: gray;");
        
        // 十六进制编码
        hexLabel = new Label(hex, this);
        hexLabel->setAlignment(Qt::AlignCenter);
        hexLabel->setStyleSheet("font-size: 10px; color: #0078d4; font-family: 'Consolas', 'Monaco', monospace;");

        vLayout->addWidget(iconLabel, 0, Qt::AlignCenter);
        vLayout->addWidget(nameLabel, 0, Qt::AlignCenter);
        vLayout->addWidget(hexLabel, 0, Qt::AlignCenter);
    }

private:
    QLabel* iconLabel;
    Label* nameLabel;
    Label* hexLabel;
};

class IconFontTestWindow : public QWidget, public FluentElement {
public:
    IconFontTestWindow(const QString& iconFamily, const QString& uiFamily) 
        : m_iconFamily(iconFamily), m_uiFamily(uiFamily) {
        setFixedSize(900, 700);
        setWindowTitle("Segoe & Fluent Icons Visual Test");
        
        auto* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(20, 20, 20, 20);
        mainLayout->setSpacing(20);

        // 1. 顶部标题
        auto* titleLabel = new Label("Iconography & Typography", this);
        titleLabel->setStyleSheet(QString("font-size: 28px; font-weight: 600; font-family: '%1';").arg(m_uiFamily));
        mainLayout->addWidget(titleLabel);

        // 2. 字体信息栏
        auto* infoLabel = new Label(QString("Icon Font: %1 | UI Font: %2").arg(m_iconFamily, m_uiFamily), this);
        mainLayout->addWidget(infoLabel);

        // 3. 图标展示滚动区
        auto* scroll = new QScrollArea(this);
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->viewport()->setAutoFillBackground(false);
        scroll->setStyleSheet("background: transparent; border: none;");
        
        auto* content = new QWidget();
        content->setObjectName("scrollContent");
        auto* grid = new QGridLayout(content);
        grid->setSpacing(10);

        // 参考 WinUI 3 Gallery 的常用图标
        struct GlyphInfo { QString glyph; QString name; QString hex; };
        QVector<GlyphInfo> glyphs = {
            { QChar(0xE700), "GlobalNav", "E700" },
            { QChar(0xE701), "Wifi", "E701" },
            { QChar(0xE702), "Bluetooth", "E702" },
            { QChar(0xE710), "Add", "E710" },
            { QChar(0xE711), "Cancel", "E711" },
            { QChar(0xE712), "More", "E712" },
            { QChar(0xE713), "Settings", "E713" },
            { QChar(0xE721), "Search", "E721" },
            { QChar(0xE80F), "Home", "E80F" },
            { QChar(0xE734), "Favorite", "E734" },
            { QChar(0xE74D), "Delete", "E74D" },
            { QChar(0xE74E), "Save", "E74E" },
            { QChar(0xE72A), "Forward", "E72A" },
            { QChar(0xE72B), "Back", "E72B" },
            { QChar(0xE76C), "ChevronRight", "E76C" },
            { QChar(0xE76B), "ChevronDown", "E76B" },
            { QChar(0xE7E8), "Light", "E7E8" },
            { QChar(0xE708), "Microphone", "E708" },
            { QChar(0xE722), "Video", "E722" },
            { QChar(0xE790), "Emoji", "E790" }
        };

        int row = 0, col = 0;
        for (const auto& g : glyphs) {
            IconCell* cell = new IconCell(g.glyph, g.name, g.hex, content);
            // 确保 Cell 使用正确的图标字体
            cell->findChild<QLabel*>()->setFont(QFont(m_iconFamily, 24));
            grid->addWidget(cell, row, col);
            if (++col > 5) { col = 0; row++; }
        }

        scroll->setWidget(content);
        mainLayout->addWidget(scroll);

        // 4. 底部控制栏
        auto* bottomBar = new QHBoxLayout();
        auto* themeBtn = new Button("Switch Theme", this);
        themeBtn->setFixedSize(120, 32);
        bottomBar->addStretch();
        bottomBar->addWidget(themeBtn);
        mainLayout->addLayout(bottomBar);

        connect(themeBtn, &Button::clicked, []() {
            FluentElement::setTheme(FluentElement::currentTheme() == Light ? Dark : Light);
        });

        onThemeUpdated();
    }

    void onThemeUpdated() override {
        const auto& c = themeColors();
        setStyleSheet(QString("QWidget#IconFontTestWindow { background-color: %1; color: %2; }")
            .arg(c.bgCanvas.name()).arg(c.textPrimary.name()));
    }

private:
    QString m_iconFamily;
    QString m_uiFamily;
};

class IconFontTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char **argv = nullptr;
        if (!qApp) new QApplication(argc, argv);
        QApplication::setStyle("Fusion");

        Q_INIT_RESOURCE(resources);

        auto loadFont = [](const QString& path) -> QString {
            int id = QFontDatabase::addApplicationFont(path);
            if (id != -1) {
                return QFontDatabase::applicationFontFamilies(id).first();
            }
            return "";
        };

        m_iconFamily = loadFont(":/res/Segoe Fluent Icons.ttf");
        m_uiFamily = loadFont(":/res/SegoeUI-VF.ttf");
        
        if (m_iconFamily.isEmpty()) m_iconFamily = "Segoe Fluent Icons";
        if (m_uiFamily.isEmpty()) m_uiFamily = "Segoe UI";
    }

    static QString m_iconFamily;
    static QString m_uiFamily;
};

QString IconFontTest::m_iconFamily = "";
QString IconFontTest::m_uiFamily = "";

TEST_F(IconFontTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("QT_QPA_PLATFORM") && qEnvironmentVariable("QT_QPA_PLATFORM") == "offscreen") {
        GTEST_SKIP() << "Skipping visual test in offscreen mode";
    }

    IconFontTestWindow window(m_iconFamily, m_uiFamily);
    window.setObjectName("IconFontTestWindow");
    window.show();
    qApp->exec();
}
