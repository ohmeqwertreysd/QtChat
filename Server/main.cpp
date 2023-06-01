
#include <QCoreApplication>
#include <QTcpServer>
#include <iostream>
#include "server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    std::cout << "Im a Server" << std::endl;
    Server server;
    return a.exec();
}
