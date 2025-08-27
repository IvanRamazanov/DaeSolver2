#include <QApplication>
#include "DaeSolver.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    
    dae_solver::DaeSolver mwind(argc, argv);
    mwind.show();

    return app.exec();
}
