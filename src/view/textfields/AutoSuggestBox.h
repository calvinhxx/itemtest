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
    bool isSuggestionListOpen() const;

    void setHeader(const QString& header);
    void setQueryIconGlyph(const QString& glyph);
    void setQueryIconVisible(bool visible);

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
    QRect inputRect() const;
    int inputTop() const;
    int totalPreferredHeight() const;

    void initializeButtons();
    void updateButtonGeometry();
    void updateButtonState();
    void updateTextMargins();
    void updateHeaderTextMargins();
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

    static constexpr int kInputHeight = 32;
    static constexpr int kHeaderHeight = 20;
    static constexpr int kHeaderGap = 4;
    static constexpr int kButtonWidth = 24;
    static constexpr int kButtonHeight = 24;
    static constexpr int kButtonRightMargin = 4;
    static constexpr int kTextButtonGap = 2;
};

} // namespace view::textfields

#endif // AUTOSUGGESTBOX_H