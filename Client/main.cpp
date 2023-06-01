
#include "chat.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("QtChat");
    Chat chat;
    chat.show();
    return a.exec();
}
