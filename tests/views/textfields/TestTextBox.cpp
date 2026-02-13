#include <gtest/gtest.h>
#include <QApplication>
#include "view/textfields/TextBox.h"
#include <QVBoxLayout>

using namespace view::textfields;

class TextBoxTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char **argv = nullptr;
        if (!qApp) {
            new QApplication(argc, argv);
        }
    }

    void SetUp() override {
        container = new QWidget();
        textBox = new TextBox(container);
    }

    void TearDown() override {
        delete container;
    }

    QWidget* container;
    TextBox* textBox;
};

TEST_F(TextBoxTest, InitialState) {
    EXPECT_TRUE(textBox->text().isEmpty());
    EXPECT_TRUE(textBox->placeholderText().isEmpty());
    EXPECT_TRUE(textBox->header().isEmpty());
    EXPECT_FALSE(textBox->isReadOnly());
    EXPECT_FALSE(textBox->isMultiLine());
}

TEST_F(TextBoxTest, SetTextAndPlaceholder) {
    QString txt = "Hello World";
    textBox->setText(txt);
    EXPECT_EQ(textBox->text(), txt);

    QString ph = "Enter text here";
    textBox->setPlaceholderText(ph);
    EXPECT_EQ(textBox->placeholderText(), ph);
}

TEST_F(TextBoxTest, HeaderVisibility) {
    // Initially empty -> hidden (implementation detail, can check layout count or visibility if possible)
    textBox->setHeader("My Title");
    EXPECT_EQ(textBox->header(), "My Title");
    
    // Check if Header widget exists
    auto children = textBox->findChildren<TextBlock*>();
    ASSERT_FALSE(children.isEmpty());
    // Use isHidden() because isVisible() returns false if parent is hidden
    EXPECT_FALSE(children.first()->isHidden());
    
    textBox->setHeader("");
    EXPECT_TRUE(children.first()->isHidden());
}

TEST_F(TextBoxTest, ReadOnlyState) {
    textBox->setReadOnly(true);
    EXPECT_TRUE(textBox->isReadOnly());
    textBox->setReadOnly(false);
    EXPECT_FALSE(textBox->isReadOnly());
}

TEST_F(TextBoxTest, MultiLineSwitch) {
    // 1. Set text in single line
    QString singleLineText = "Line 1";
    textBox->setText(singleLineText);
    
    // 2. Switch to MultiLine
    textBox->setMultiLine(true);
    EXPECT_TRUE(textBox->isMultiLine());
    EXPECT_EQ(textBox->text(), singleLineText); // Text should be preserved

    // 3. Edit in MultiLine
    QString multiLineText = "Line 1\nLine 2";
    textBox->setText(multiLineText);
    EXPECT_EQ(textBox->text(), multiLineText);

    // 4. Switch back to Single Line (Note: QLineEdit might not support newlines well, behavior test)
    textBox->setMultiLine(false);
    EXPECT_FALSE(textBox->isMultiLine());
    // Expect text to be preserved, even if newlines are stripped or displayed weirdly
    // QPlainTextEdit -> QLineEdit conversion usually keeps text but strictly speaking QLineEdit doesn't display \n.
    // However, setText() on QLineEdit with \n works but displays symbols.
    // Let's just check equality of string content.
    EXPECT_EQ(textBox->text(), multiLineText); 
}

TEST_F(TextBoxTest, VisualGallery) {
    // This test constructs a UI for manual visual verification (if run with GUI)
    QWidget* window = new QWidget();
    window->setWindowTitle("TextBox Gallery");
    window->resize(400, 600);
    auto* layout = new QVBoxLayout(window);

    // 1. Simple
    auto* t1 = new TextBox(window);
    t1->setPlaceholderText("Simple TextBox");
    layout->addWidget(t1);

    // 2. With Header
    auto* t2 = new TextBox(window);
    t2->setHeader("Username:");
    t2->setPlaceholderText("Enter username");
    layout->addWidget(t2);

    // 3. MultiLine
    auto* t3 = new TextBox(window);
    t3->setHeader("Comments:");
    t3->setMultiLine(true);
    t3->setPlaceholderText("Write your feedback...");
    t3->setFixedHeight(100); // Manual height for multiline
    layout->addWidget(t3);

    // 4. ReadOnly
    auto* t4 = new TextBox(window);
    t4->setText("I am read only");
    t4->setReadOnly(true);
    layout->addWidget(t4);
    
    window->show();
    qApp->exec(); // Blocking call, uncomment for manual check

    delete window;
}
