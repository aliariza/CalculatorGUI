#ifndef CALCULATOR_WIDGET_H
#define CALCULATOR_WIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include "calculator.h"

class CalculatorWidget : public QWidget {
    Q_OBJECT
public:
    explicit CalculatorWidget(QWidget* parent = nullptr);


protected:
    // ✅ This allows keyboard input
    void keyPressEvent(QKeyEvent* event) override;

private:
    Calculator calc_;
    QLineEdit* display_=nullptr;
    QLabel* status_=nullptr;
    QLabel* memIndicator_=nullptr;


    void addButton(class QGridLayout* grid, const QString& text, int row, int col, int rowSpan = 1, int colSpan = 1);
    void refresh();
    void handleKey(const QString& text);

};

#endif
