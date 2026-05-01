#ifndef AUTOSUGGESTBOX_H
#define AUTOSUGGESTBOX_H

#include "LineEdit.h"

#include <QMargins>
#include <QStringList>
#include <QVariant>

class QKeyEvent;
class QMoveEvent;
class QPainter;

namespace view::basicinput { class Button; }

namespace view::textfields {

class SuggestionListPopup;

class AutoSuggestBox : public LineEdit {
    Q_OBJECT
    Q_PROPERTY(QStringList suggestions READ suggestions WRITE setSuggestions NOTIFY suggestionsChanged)
    Q_PROPERTY(QString header READ header WRITE setHeader NOTIFY headerChanged)
    Q_PROPERTY(QString queryIconGlyph READ queryIconGlyph WRITE setQueryIconGlyph NOTIFY queryIconGlyphChanged)
    Q_PROPERTY(bool queryIconVisible READ isQueryIconVisible WRITE setQueryIconVisible NOTIFY queryIconVisibleChanged)
    Q_PROPERTY(int inputHeight READ inputHeight WRITE setInputHeight NOTIFY inputHeightChanged)
    Q_PROPERTY(int queryButtonSize READ queryButtonSize WRITE setQueryButtonSize NOTIFY queryButtonSizeChanged)
    Q_PROPERTY(int clearButtonSize READ clearButtonSize WRITE setClearButtonSize NOTIFY clearButtonSizeChanged)
    Q_PROPERTY(QString suggestionFontRole READ suggestionFontRole WRITE setSuggestionFontRole NOTIFY suggestionFontRoleChanged)
    Q_PROPERTY(int suggestionItemHeight READ suggestionItemHeight WRITE setSuggestionItemHeight NOTIFY suggestionItemHeightChanged)
    Q_PROPERTY(bool isSuggestionListOpen READ isSuggestionListOpen NOTIFY suggestionListOpenChanged)

public:
    enum class TextChangeReason {
        UserInput,
        ProgrammaticChange,
        SuggestionChosen
    };
    Q_ENUM(TextChangeReason)

    explicit AutoSuggestBox(QWidget* parent = nullptr);
    ~AutoSuggestBox() override;

    QStringList suggestions() const { return m_suggestions; }
    QString header() const { return m_header; }
    QString queryIconGlyph() const { return m_queryIconGlyph; }
    bool isQueryIconVisible() const { return m_queryIconVisible; }
    int inputHeight() const { return m_inputHeight; }
    int queryButtonSize() const { return m_queryButtonSize; }
    int clearButtonSize() const { return m_clearButtonSize; }
    QString suggestionFontRole() const { return m_suggestionFontRole; }
    int suggestionItemHeight() const { return m_suggestionItemHeight; }
    bool isSuggestionListOpen() const;

    void setHeader(const QString& header);
    void setQueryIconGlyph(const QString& glyph);
    void setQueryIconVisible(bool visible);
    void setInputHeight(int height);
    void setQueryButtonSize(int size);
    void setClearButtonSize(int size);
    void setSuggestionFontRole(const QString& role);
    void setSuggestionItemHeight(int height);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void onThemeUpdated() override;

public slots:
    void setSuggestions(const QStringList& suggestions);
    void clearSuggestions();

signals:
    void textChangedWithReason(const QString& text, AutoSuggestBox::TextChangeReason reason);
    void suggestionChosen(const QVariant& item);
    void querySubmitted(const QString& queryText, const QVariant& chosenSuggestion);
    void suggestionsChanged();
    void headerChanged();
    void queryIconGlyphChanged();
    void queryIconVisibleChanged();
    void inputHeightChanged();
    void queryButtonSizeChanged();
    void suggestionFontRoleChanged();
    void suggestionItemHeightChanged();
    void suggestionListOpenChanged(bool open);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void enterEvent(FluentEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    static constexpr int kDefaultInputHeight = 32;
    static constexpr int kDefaultSuggestionItemHeight = 40;
    static constexpr int kHeaderHeight = 20;
    static constexpr int kHeaderGap = 4;
    static constexpr int kButtonRightMargin = 4;
    static constexpr int kTextButtonGap = 2;

    QRect inputRect() const;
    int inputTop() const;
    int totalPreferredHeight() const;
    int inputTextVerticalPadding() const;

    void initializeButtons();
    void updateButtonGeometry();
    void updateButtonState();
    void updateTextMargins();
    void updateHeaderTextMargins();
    void updateSuggestionMetrics();
    void handleTextChanged(const QString& text);

    void openSuggestionList();
    void closeSuggestionList();
    void previewSuggestion(int row);
    void chooseSuggestion(int row);
    void setPopupCurrentRow(int row);
    void setTextWithReason(const QString& value, TextChangeReason reason, bool emitWhenUnchanged = false);

    void paintInputFrame(QPainter& painter);
    void paintHeader(QPainter& painter);

    QStringList m_suggestions;
    QString m_header;
    QString m_queryIconGlyph = QString::fromUtf16(u"\uE721");
    bool m_queryIconVisible = true;

    ::view::basicinput::Button* m_queryButton = nullptr;
    ::view::basicinput::Button* m_clearButton = nullptr;
    ::view::AnchorLayout* m_buttonLayout = nullptr;
    SuggestionListPopup* m_suggestionPopup = nullptr;

    TextChangeReason m_nextChangeReason = TextChangeReason::ProgrammaticChange;
    QString m_userTypedText;
    bool m_hovered = false;
    bool m_focused = false;

    int m_inputHeight = kDefaultInputHeight;
    int m_queryButtonSize = 24;
    int m_clearButtonSize = 24;
    QString m_suggestionFontRole = Typography::FontRole::Body;
    int m_suggestionItemHeight = kDefaultSuggestionItemHeight;
};

} // namespace view::textfields

#endif // AUTOSUGGESTBOX_H
