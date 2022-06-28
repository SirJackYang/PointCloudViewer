#include "PointCloudViewer.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PointCloudViewer w;
//    w.SetAModelFile(argv[1]);
    w.show();
    return a.exec();
}
