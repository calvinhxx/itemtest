#include <gtest/gtest.h>
#include <QApplication>
#include <QPushButton>
#include <QTimer>
#include <QStyle>
#include <QGroupBox>
#include "view/basicinput/Button.h"
#include "view/QMLPlus.h"
#include "view/FluentElement.h"
#include "view/textfields/Label.h"

using namespace view::basicinput;
using namespace view::textfields;
using namespace view;

class FluentTestWindow : public QWidget, public FluentElement {
public:
    using QWidget::QWidget;
    void onThemeUpdated() override {
        const auto& c = themeColors();
        setStyleSheet(QString("background-color: %1;").arg(c.bgCanvas.name()));
    }
};

class ButtonTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char **argv = nullptr;
        if (!qApp) new QApplication(argc, argv);
        QApplication::setStyle("Fusion");
    }

    void SetUp() override {
        window = new FluentTestWindow();
        window->setFixedSize(600, 750);
        window->setWindowTitle("Button Properties Comprehensive Test");
        layout = new AnchorLayout(window);
        window->setLayout(layout);
        window->onThemeUpdated();
    }

    void TearDown() override {
        delete window;
    }

    FluentTestWindow* window;
    AnchorLayout* layout;
};

TEST_F(ButtonTest, VisualPropertyVerification) {
    if (qEnvironmentVariableIsSet("QT_QPA_PLATFORM") && qEnvironmentVariable("QT_QPA_PLATFORM") == "offscreen") {
        GTEST_SKIP() << "Skipping visual test in offscreen mode";
    }

    using Edge = AnchorLayout::Edge;

    auto createLabel = [&](const QString& text, QWidget* anchor, int topMargin = 30) {
        Label* l = new Label(text, window);
        l->anchors()->top = {anchor, Edge::Bottom, topMargin};
        l->anchors()->left = {window, Edge::Left, 40};
        layout->addWidget(l);
        return l;
    };

    // --- 1. Style Test (Standard, Accent, Subtle) ---
    Label* lblStyle = new Label("1. Button Styles:", window);
    lblStyle->anchors()->top = {window, Edge::Top, 20};
    lblStyle->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(lblStyle);

    Button* btnStd = new Button("Standard", window);
    btnStd->setFluentStyle(Button::Standard);
    btnStd->setFixedSize(120, 32);
    btnStd->anchors()->top = {lblStyle, Edge::Bottom, 10};
    btnStd->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(btnStd);

    Button* btnAccent = new Button("Accent", window);
    btnAccent->setFluentStyle(Button::Accent);
    btnAccent->setFixedSize(120, 32);
    btnAccent->anchors()->verticalCenter = {btnStd, Edge::VCenter, 0};
    btnAccent->anchors()->left = {btnStd, Edge::Right, 20};
    layout->addWidget(btnAccent);

    Button* btnSubtle = new Button("Subtle", window);
    btnSubtle->setFluentStyle(Button::Subtle);
    btnSubtle->setFixedSize(120, 32);
    btnSubtle->anchors()->verticalCenter = {btnStd, Edge::VCenter, 0};
    btnSubtle->anchors()->left = {btnAccent, Edge::Right, 20};
    layout->addWidget(btnSubtle);

    // --- 2. Size Test (Small, Standard, Large) ---
    Label* lblSize = createLabel("2. Button Sizes:", btnStd);
    
    Button* btnSmall = new Button("Small", window);
    btnSmall->setFluentSize(Button::Small);
    btnSmall->anchors()->top = {lblSize, Edge::Bottom, 10};
    btnSmall->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(btnSmall);

    Button* btnNormalSize = new Button("Standard", window);
    btnNormalSize->setFluentSize(Button::StandardSize);
    btnNormalSize->anchors()->verticalCenter = {btnSmall, Edge::VCenter, 0};
    btnNormalSize->anchors()->left = {btnSmall, Edge::Right, 20};
    layout->addWidget(btnNormalSize);

    Button* btnLarge = new Button("Large Size", window);
    btnLarge->setFluentSize(Button::Large);
    btnLarge->anchors()->verticalCenter = {btnSmall, Edge::VCenter, 0};
    btnLarge->anchors()->left = {btnNormalSize, Edge::Right, 20};
    layout->addWidget(btnLarge);

    // --- 3. Layout Test (TextOnly, IconBefore, IconOnly, IconAfter) ---
    Label* lblLayout = createLabel("3. Content Layouts:", btnLarge);
    QIcon icon = window->style()->standardIcon(QStyle::SP_ComputerIcon);

    Button* l1 = new Button("Text Only", window);
    l1->setFluentLayout(Button::TextOnly);
    l1->anchors()->top = {lblLayout, Edge::Bottom, 10};
    l1->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(l1);

    Button* l2 = new Button("Icon Before", window);
    l2->setIcon(icon);
    l2->setFluentLayout(Button::IconBefore);
    l2->anchors()->verticalCenter = {l1, Edge::VCenter, 0};
    l2->anchors()->left = {l1, Edge::Right, 20};
    layout->addWidget(l2);

    Button* l3 = new Button("", window);
    l3->setIcon(icon);
    l3->setFluentLayout(Button::IconOnly);
    l3->setFixedSize(40, 40);
    l3->anchors()->verticalCenter = {l1, Edge::VCenter, 0};
    l3->anchors()->left = {l2, Edge::Right, 20};
    layout->addWidget(l3);

    Button* l4 = new Button("Icon After", window);
    l4->setIcon(icon);
    l4->setFluentLayout(Button::IconAfter);
    l4->anchors()->verticalCenter = {l1, Edge::VCenter, 0};
    l4->anchors()->left = {l3, Edge::Right, 20};
    layout->addWidget(l4);

    // --- 4. Interaction States (Forced) ---
    Label* lblStates = createLabel("4. Forced Interaction States:", l3, 40);

    Button* sRest = new Button("Rest", window);
    sRest->setInteractionState(Button::Rest);
    sRest->anchors()->top = {lblStates, Edge::Bottom, 10};
    sRest->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(sRest);

    Button* sHover = new Button("Forced Hover", window);
    sHover->setInteractionState(Button::Hover);
    sHover->anchors()->verticalCenter = {sRest, Edge::VCenter, 0};
    sHover->anchors()->left = {sRest, Edge::Right, 20};
    layout->addWidget(sHover);

    Button* sPressed = new Button("Forced Pressed", window);
    sPressed->setInteractionState(Button::Pressed);
    sPressed->anchors()->verticalCenter = {sRest, Edge::VCenter, 0};
    sPressed->anchors()->left = {sHover, Edge::Right, 20};
    layout->addWidget(sPressed);

    Button* sDisabled = new Button("Forced Disabled", window);
    sDisabled->setInteractionState(Button::Disabled);
    sDisabled->anchors()->verticalCenter = {sRest, Edge::VCenter, 0};
    sDisabled->anchors()->left = {sPressed, Edge::Right, 20};
    layout->addWidget(sDisabled);

    // --- 5. Focus Visual & Dynamic State ---
    Label* lblFocus = createLabel("5. Focus Visual & State Transition:", sRest, 40);

    Button* focusBtn = new Button("Forced Focus Visual", window);
    focusBtn->setFocusVisual(true);
    focusBtn->setFluentStyle(Button::Accent);
    focusBtn->setFixedSize(200, 40);
    focusBtn->anchors()->top = {lblFocus, Edge::Bottom, 10};
    focusBtn->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(focusBtn);

    Button* stateToggle = new Button("Toggle Dynamic State", window);
    stateToggle->setFixedSize(200, 40);
    stateToggle->anchors()->verticalCenter = {focusBtn, Edge::VCenter, 0};
    stateToggle->anchors()->left = {focusBtn, Edge::Right, 30};
    layout->addWidget(stateToggle);

    QMLState active;
    active.name = "active";
    active.changes = {
        { stateToggle, "text", "ACTIVE STATE" },
        { stateToggle, "fluentStyle", Button::Accent },
        { stateToggle, "focusVisual", true }
    };
    stateToggle->addState(active);

    QObject::connect(stateToggle, &Button::clicked, [stateToggle]() {
        stateToggle->setState(stateToggle->state() == "" ? "active" : "");
    });

    // --- Theme Switcher ---
    Button* themeBtn = new Button("Switch Theme", window);
    themeBtn->setFixedSize(120, 32);
    themeBtn->anchors()->bottom = {window, Edge::Bottom, -30};
    themeBtn->anchors()->right = {window, Edge::Right, -40};
    layout->addWidget(themeBtn);

    QObject::connect(themeBtn, &Button::clicked, []() {
        FluentElement::setTheme(FluentElement::currentTheme() == FluentElement::Light ? FluentElement::Dark : FluentElement::Light);
    });

    window->show();
    qApp->exec();
}
