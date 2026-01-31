#include "calculator_widget.h"

#include <QGridLayout>
#include <QPushButton>
#include <QFont>
#include <QStyle>
#include <QKeyEvent>



static const char* kAppStyle = R"(
QWidget {
    background: #0f1115;
    color: #e6e6e6;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial;
}

QLineEdit {
    background: #161a22;
    border: 1px solid #232a36;
    border-radius: 16px;
    padding: 14px 14px;
    color: #eaf2ff;
    selection-background-color: #2f81f7;
}

QLabel {
    color: #9aa4b2;
}
QLabel#memIndicator {
    color: #1f5eff;
}
/* Base button */
QPushButton {
    background: #1b2230;
    border: 1px solid #252f3f;
    border-radius: 16px;
    padding: 10px;
    color: #e6e6e6;
}

QPushButton:hover {
    background: #222c3d;
}

QPushButton:pressed {
    background: #131a26;
}

/* Number buttons */
QPushButton[number="true"] {
    background: #18202d;
}
QPushButton[number="true"]:hover {
    background: #1f2a3b;
}

/* Operator buttons */
QPushButton[op="true"] {
    background: #2a1f12;
    border: 1px solid #3a2a18;
    color: #ffd7a3;
}
QPushButton[op="true"]:hover {
    background: #332513;
}

/* Equals button */
QPushButton[eq="true"] {
    background: #1f5eff;
    border: 1px solid #1f5eff;
    color: #ffffff;
    font-weight: 700;
}
QPushButton[eq="true"]:hover {
    background: #2b6bff;
}

/* Clear / Backspace */
QPushButton[util="true"] {
    background: #2b2f3a;
    border: 1px solid #3a4150;
    color: #e6e6e6;
}
QPushButton[util="true"]:hover {
    background: #333949;
}

/* Error state for display */
QLineEdit[err="true"] {
    color: #ff6b6b;
    border: 1px solid #5a2a2a;
}
)";


CalculatorWidget::CalculatorWidget(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle("Calculator (C++20)");
    setMinimumSize(360, 520);

    setStyleSheet(kAppStyle);
    setAutoFillBackground(true);


    auto* layout = new QGridLayout(this);
    layout->setSpacing(12);
    layout->setContentsMargins(18, 18, 18, 18);

    display_ = new QLineEdit(this);
    display_->setReadOnly(true);
    display_->setAlignment(Qt::AlignRight);
    display_->setMinimumHeight(70);

    QFont df = display_->font();
    df.setPointSize(28);
    df.setBold(true);
    display_->setFont(df);

    status_ = new QLabel(this);
    status_->setAlignment(Qt::AlignRight);
    status_->setMinimumHeight(22);
    memIndicator_ = new QLabel(this);
    memIndicator_->setObjectName("memIndicator");

    memIndicator_->setMinimumHeight(22);
    memIndicator_->setAlignment(Qt::AlignLeft);
    memIndicator_->setText(""); // empty initially

    QFont mf = memIndicator_->font();
    mf.setBold(true);
    memIndicator_->setFont(mf);
    layout->addWidget(display_, 0, 0, 1, 4);
    layout->addWidget(memIndicator_, 1, 0, 1, 1);
    layout->addWidget(status_, 1, 1, 1, 4);
    // Buttons
    // Memory + percent row
    addButton(layout, "MC", 2, 0);
    addButton(layout, "MR", 2, 1);
    addButton(layout, "M+", 2, 2);
    addButton(layout, "%",  2, 3);

    // 7 8 9 /
    addButton(layout, "7", 3, 0);
    addButton(layout, "8", 3, 1);
    addButton(layout, "9", 3, 2);
    addButton(layout, "/", 3, 3);

    // 4 5 6 *
    addButton(layout, "4", 4, 0);
    addButton(layout, "5", 4, 1);
    addButton(layout, "6", 4, 2);
    addButton(layout, "*", 4, 3);

    // 1 2 3 -
    addButton(layout, "1", 5, 0);
    addButton(layout, "2", 5, 1);
    addButton(layout, "3", 5, 2);
    addButton(layout, "-", 5, 3);

    // C 0 ⌫ +
    addButton(layout, "C", 6, 0);
    addButton(layout, "0", 6, 1);
    addButton(layout, ".", 6, 2);   // ✅ decimal point
    addButton(layout, "+", 6, 3);
    // = spans full width
    addButton(layout, "=", 7, 0, 1, 3);
    addButton(layout, "⌫", 7, 3, 1, 1);   // move backspace above +


    // Make columns stretch evenly
    for (int c = 0; c < 4; ++c) layout->setColumnStretch(c, 1);

    refresh();
}


void CalculatorWidget::addButton(QGridLayout* grid, const QString& text, int row, int col, int rowSpan, int colSpan) {
    auto* btn = new QPushButton(text, this);
    btn->setMinimumHeight(56);

    QFont bf = btn->font();
    bf.setPointSize(18);
    bf.setBold(true);
    btn->setFont(bf);

    // Tag buttons for styling (dynamic properties)
    if (text.size() == 1 && text[0].isDigit()) {
        btn->setProperty("number", true);
    } else if (text == "=") {
        btn->setProperty("eq", true);
    } else if (text == "C" || text == "⌫" || text == "MC" || text == "MR" || text == "M+" || text == "%") {
        btn->setProperty("util", true);
    } else {
        // + - * /
        btn->setProperty("op", true);
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }

    QObject::connect(btn, &QPushButton::clicked, this, [this, text]() {
        handleKey(text);
    });

    grid->addWidget(btn, row, col, rowSpan, colSpan);
}


void CalculatorWidget::handleKey(const QString& text) {
    if (text == "C") calc_.press('c');
    else if (text == "MC") calc_.press('X');
    else if (text == "MR") calc_.press('R');
    else if (text == "M+") calc_.press('M');
    else if (text == "%") calc_.press('%');
    else if (text == "⌫") calc_.press('b');
    else if (text == "=") calc_.press('=');
    else if (text.size() == 1) {
        const QChar ch = text[0];
        const char c = ch.toLatin1();
        // digits and operators
        calc_.press(c);
    }
    refresh();
}

void CalculatorWidget::refresh() {
    display_->setText(QString::fromStdString(calc_.display()));
    status_->setText(QString::fromStdString(calc_.statusLine()));
    if (memIndicator_) {
        memIndicator_->setText(calc_.hasMemory() ? "M" : "");
    }
    display_->setProperty("err", calc_.hasError());
    // Re-apply stylesheet after property change
    display_->style()->unpolish(display_);
    display_->style()->polish(display_);
}
void CalculatorWidget::keyPressEvent(QKeyEvent* event) {
    int k = event->key();

    if (k >= Qt::Key_0 && k <= Qt::Key_9)
        calc_.press('0' + (k - Qt::Key_0));

    else if (k == Qt::Key_Plus) calc_.press('+');
    else if (k == Qt::Key_Minus) calc_.press('-');
    else if (k == Qt::Key_Asterisk) calc_.press('*');
    else if (k == Qt::Key_Slash) calc_.press('/');
    else if (k == Qt::Key_Enter || k == Qt::Key_Return) calc_.press('=');
    else if (k == Qt::Key_Backspace) calc_.press('b');
    else if (k == Qt::Key_Escape) calc_.press('c');

    refresh();
}
