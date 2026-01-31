#include <QApplication>
#include <QStyleFactory>
#include "calculator_widget.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // ✅ Force Qt Fusion style (allows full stylesheet control)
app.setStyle(QStyleFactory::create("Fusion"));

    CalculatorWidget w;
    w.show();

    return app.exec();
}
