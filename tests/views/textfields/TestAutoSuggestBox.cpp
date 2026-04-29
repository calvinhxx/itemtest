#include <gtest/gtest.h>

#include <QApplication>
#include <QFontDatabase>
#include <QTimer>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include "design/Typography.h"
#include "view/QMLPlus.h"
#include "view/basicinput/Button.h"
#include "view/textfields/AutoSuggestBox.h"
#include "view/textfields/TextBlock.h"

using namespace view;
using namespace view::basicinput;
using namespace view::textfields;

class AutoSuggestBoxTestWindow : public QWidget, public FluentElement {
public:
    using QWidget::QWidget;
    void onThemeUpdated() override {
        const auto& colors = themeColors();
        setStyleSheet(QString("background-color: %1;").arg(colors.bgCanvas.name()));
    }
};

class AutoSuggestBoxTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char** argv = nullptr;
        if (!qApp) new QApplication(argc, argv);
        QApplication::setStyle("Fusion");

        Q_INIT_RESOURCE(resources);
        QFontDatabase::addApplicationFont(":/res/Segoe Fluent Icons.ttf");
        QFontDatabase::addApplicationFont(":/res/SegoeUI-VF.ttf");
        qRegisterMetaType<AutoSuggestBox::TextChangeReason>("AutoSuggestBox::TextChangeReason");
    }

    void SetUp() override {
        window = new AutoSuggestBoxTestWindow();
        window->setFixedSize(520, 360);
        layout = new AnchorLayout(window);
        window->setLayout(layout);
        window->onThemeUpdated();
    }

    void TearDown() override {
        delete window;
    }

    void showAndFocus(AutoSuggestBox* box) {
        window->show();
        box->setFocus(Qt::OtherFocusReason);
        QApplication::processEvents();
    }

    AutoSuggestBoxTestWindow* window = nullptr;
    AnchorLayout* layout = nullptr;
};

TEST_F(AutoSuggestBoxTest, DefaultsAndButtons) {
    AutoSuggestBox box(window);

    EXPECT_TRUE(box.suggestions().isEmpty());
    EXPECT_TRUE(box.header().isEmpty());
    EXPECT_TRUE(box.isQueryIconVisible());
    EXPECT_EQ(box.queryIconGlyph(), Typography::Icons::Search);
    EXPECT_FALSE(box.isSuggestionListOpen());
    EXPECT_EQ(box.sizeHint().height(), 32);

    auto* queryButton = box.findChild<Button*>("AutoSuggestBoxQueryButton");
    auto* clearButton = box.findChild<Button*>("AutoSuggestBoxClearButton");
    ASSERT_NE(queryButton, nullptr);
    ASSERT_NE(clearButton, nullptr);
    EXPECT_FALSE(queryButton->isHidden());
    EXPECT_TRUE(clearButton->isHidden());
}

TEST_F(AutoSuggestBoxTest, PropertySettersEmitSignals) {
    AutoSuggestBox box(window);
    QSignalSpy suggestionsSpy(&box, &AutoSuggestBox::suggestionsChanged);
    QSignalSpy headerSpy(&box, &AutoSuggestBox::headerChanged);
    QSignalSpy iconSpy(&box, &AutoSuggestBox::queryIconGlyphChanged);
    QSignalSpy iconVisibleSpy(&box, &AutoSuggestBox::queryIconVisibleChanged);

    const QStringList items{"Alpha", "Beta"};
    box.setSuggestions(items);
    box.setHeader("Search files");
    box.setQueryIconGlyph(Typography::Icons::Filter);
    box.setQueryIconVisible(false);

    EXPECT_EQ(box.suggestions(), items);
    EXPECT_EQ(box.header(), "Search files");
    EXPECT_EQ(box.queryIconGlyph(), Typography::Icons::Filter);
    EXPECT_FALSE(box.isQueryIconVisible());
    EXPECT_GT(box.sizeHint().height(), 32);

    EXPECT_EQ(suggestionsSpy.count(), 1);
    EXPECT_EQ(headerSpy.count(), 1);
    EXPECT_EQ(iconSpy.count(), 1);
    EXPECT_EQ(iconVisibleSpy.count(), 1);

    box.setSuggestions(items);
    box.setHeader("Search files");
    box.setQueryIconGlyph(Typography::Icons::Filter);
    box.setQueryIconVisible(false);
    EXPECT_EQ(suggestionsSpy.count(), 1);
    EXPECT_EQ(headerSpy.count(), 1);
    EXPECT_EQ(iconSpy.count(), 1);
    EXPECT_EQ(iconVisibleSpy.count(), 1);
}

TEST_F(AutoSuggestBoxTest, ProgrammaticAndUserTextReasons) {
    AutoSuggestBox* box = new AutoSuggestBox(window);
    box->setFixedWidth(220);
    layout->addWidget(box);

    QSignalSpy textSpy(box, &AutoSuggestBox::textChangedWithReason);
    box->setText("seed");
    ASSERT_EQ(textSpy.count(), 1);
    EXPECT_EQ(textSpy.takeFirst().at(1).value<AutoSuggestBox::TextChangeReason>(),
              AutoSuggestBox::TextChangeReason::ProgrammaticChange);

    showAndFocus(box);
    QTest::keyClicks(box, "x");
    ASSERT_GE(textSpy.count(), 1);
    EXPECT_EQ(textSpy.last().at(1).value<AutoSuggestBox::TextChangeReason>(),
              AutoSuggestBox::TextChangeReason::UserInput);
}

TEST_F(AutoSuggestBoxTest, UserInputOpensAndEscapeClosesSuggestions) {
    AutoSuggestBox* box = new AutoSuggestBox(window);
    box->setFixedWidth(220);
    box->setSuggestions({"Alpha", "Alpine", "Azure"});
    layout->addWidget(box);
    showAndFocus(box);

    QSignalSpy openSpy(box, &AutoSuggestBox::suggestionListOpenChanged);
    QTest::keyClicks(box, "a");
    QApplication::processEvents();

    EXPECT_TRUE(box->isSuggestionListOpen());
    ASSERT_GE(openSpy.count(), 1);
    EXPECT_TRUE(openSpy.first().at(0).toBool());

    QTest::keyClick(box, Qt::Key_Escape);
    QApplication::processEvents();
    EXPECT_FALSE(box->isSuggestionListOpen());
    EXPECT_FALSE(openSpy.last().at(0).toBool());
}

TEST_F(AutoSuggestBoxTest, KeyboardPreviewAndSubmitSuggestion) {
    AutoSuggestBox* box = new AutoSuggestBox(window);
    box->setFixedWidth(220);
    box->setSuggestions({"Alpha", "Alpine", "Azure"});
    layout->addWidget(box);
    showAndFocus(box);

    QSignalSpy textSpy(box, &AutoSuggestBox::textChangedWithReason);
    QSignalSpy chosenSpy(box, &AutoSuggestBox::suggestionChosen);
    QSignalSpy querySpy(box, &AutoSuggestBox::querySubmitted);

    QTest::keyClicks(box, "a");
    QApplication::processEvents();
    EXPECT_TRUE(box->isSuggestionListOpen());

    QTest::keyClick(box, Qt::Key_Down);
    QApplication::processEvents();
    ASSERT_EQ(chosenSpy.count(), 1);
    EXPECT_EQ(chosenSpy.last().at(0).toString(), "Alpha");
    EXPECT_EQ(box->text(), "Alpha");
    ASSERT_GE(textSpy.count(), 1);
    EXPECT_EQ(textSpy.last().at(1).value<AutoSuggestBox::TextChangeReason>(),
              AutoSuggestBox::TextChangeReason::ProgrammaticChange);

    QTest::keyClick(box, Qt::Key_Up);
    QApplication::processEvents();
    EXPECT_EQ(box->text(), "a");
    EXPECT_TRUE(box->isSuggestionListOpen());

    QTest::keyClick(box, Qt::Key_Down);
    QTest::keyClick(box, Qt::Key_Return);
    QApplication::processEvents();

    ASSERT_EQ(querySpy.count(), 1);
    EXPECT_EQ(querySpy.last().at(0).toString(), "Alpha");
    EXPECT_EQ(querySpy.last().at(1).toString(), "Alpha");
    EXPECT_EQ(textSpy.last().at(1).value<AutoSuggestBox::TextChangeReason>(),
              AutoSuggestBox::TextChangeReason::SuggestionChosen);
    EXPECT_FALSE(box->isSuggestionListOpen());
}

TEST_F(AutoSuggestBoxTest, QueryAndClearButtons) {
    AutoSuggestBox box(window);
    box.resize(240, box.sizeHint().height());
    QSignalSpy querySpy(&box, &AutoSuggestBox::querySubmitted);
    QSignalSpy textSpy(&box, &AutoSuggestBox::textChangedWithReason);

    auto* queryButton = box.findChild<Button*>("AutoSuggestBoxQueryButton");
    auto* clearButton = box.findChild<Button*>("AutoSuggestBoxClearButton");
    ASSERT_NE(queryButton, nullptr);
    ASSERT_NE(clearButton, nullptr);

    box.setText("hello");
    EXPECT_FALSE(clearButton->isHidden());
    EXPECT_LT(clearButton->geometry().right(), queryButton->geometry().left());
    EXPECT_EQ(queryButton->size(), QSize(24, 24));
    EXPECT_EQ(clearButton->size(), QSize(24, 24));

    QTest::keyClick(&box, Qt::Key_Return);
    ASSERT_EQ(querySpy.count(), 1);
    EXPECT_EQ(querySpy.last().at(0).toString(), "hello");
    EXPECT_FALSE(querySpy.last().at(1).isValid());

    queryButton->click();
    ASSERT_EQ(querySpy.count(), 2);
    EXPECT_EQ(querySpy.last().at(0).toString(), "hello");
    EXPECT_FALSE(querySpy.last().at(1).isValid());

    clearButton->click();
    EXPECT_TRUE(box.text().isEmpty());
    EXPECT_TRUE(clearButton->isHidden());
    ASSERT_GE(textSpy.count(), 2);
    EXPECT_EQ(textSpy.last().at(1).value<AutoSuggestBox::TextChangeReason>(),
              AutoSuggestBox::TextChangeReason::UserInput);
}

TEST_F(AutoSuggestBoxTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("SKIP_VISUAL_TEST")) {
        GTEST_SKIP() << "Set SKIP_VISUAL_TEST=1 to skip visual tests";
    }

    using Edge = AnchorLayout::Edge;
    window->setFixedSize(520, 460);

    TextBlock* title = new TextBlock("AutoSuggestBox", window);
    title->setFluentTypography(Typography::FontRole::Subtitle);
    title->anchors()->top = {window, Edge::Top, 28};
    title->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(title);

    AutoSuggestBox* rest = new AutoSuggestBox(window);
    rest->setPlaceholderText("Search");
    rest->setSuggestions({"Calendar", "Calculator", "Camera", "Clock"});
    rest->anchors()->top = {title, Edge::Bottom, 16};
    rest->anchors()->left = {window, Edge::Left, 40};
    rest->anchors()->right = {window, Edge::Right, -40};
    layout->addWidget(rest);

    AutoSuggestBox* typingFlyout = new AutoSuggestBox(window);
    typingFlyout->setPlaceholderText("Focused typing with flyout");
    typingFlyout->setSuggestions({"Calendar", "Calculator", "Camera", "Clock"});
    typingFlyout->anchors()->top = {rest, Edge::Bottom, 16};
    typingFlyout->anchors()->left = {window, Edge::Left, 40};
    typingFlyout->anchors()->right = {window, Edge::Right, -40};
    layout->addWidget(typingFlyout);

    AutoSuggestBox* withText = new AutoSuggestBox(window);
    withText->setText("settings");
    withText->setSuggestions({"Settings", "Sound settings", "Display settings"});
    withText->anchors()->top = {typingFlyout, Edge::Bottom, 20};
    withText->anchors()->left = {window, Edge::Left, 40};
    withText->anchors()->right = {window, Edge::Right, -40};
    layout->addWidget(withText);

    AutoSuggestBox* withHeader = new AutoSuggestBox(window);
    withHeader->setHeader("Command");
    withHeader->setPlaceholderText("Type a command");
    withHeader->setSuggestions({"Open", "Save", "Share"});
    withHeader->anchors()->top = {withText, Edge::Bottom, 20};
    withHeader->anchors()->left = {window, Edge::Left, 40};
    withHeader->anchors()->right = {window, Edge::Right, -40};
    layout->addWidget(withHeader);

    AutoSuggestBox* disabled = new AutoSuggestBox(window);
    disabled->setText("Disabled state");
    disabled->setEnabled(false);
    disabled->anchors()->top = {withHeader, Edge::Bottom, 20};
    disabled->anchors()->left = {window, Edge::Left, 40};
    disabled->anchors()->right = {window, Edge::Right, -40};
    layout->addWidget(disabled);

    Button* themeButton = new Button("Switch Theme", window);
    themeButton->setFluentStyle(Button::Accent);
    themeButton->setFixedSize(120, 32);
    themeButton->anchors()->bottom = {window, Edge::Bottom, -28};
    themeButton->anchors()->right = {window, Edge::Right, -40};
    layout->addWidget(themeButton);

    QObject::connect(themeButton, &Button::clicked, []() {
        FluentElement::setTheme(FluentElement::currentTheme() == FluentElement::Light
            ? FluentElement::Dark : FluentElement::Light);
    });

    window->show();
    QTimer::singleShot(0, [typingFlyout]() {
        typingFlyout->setFocus();
        QTest::keyClicks(typingFlyout, "c");
    });
    qApp->exec();
}