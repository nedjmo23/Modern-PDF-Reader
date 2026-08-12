#include "ModernPDFReader.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ModernPDFReader viewer;
    viewer.show();
    return app.exec();
}
