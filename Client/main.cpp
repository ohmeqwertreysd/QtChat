
#include "chat.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Chat chat;
    chat.show();
    return a.exec();
}
