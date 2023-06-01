#ifndef CHAT_H
#define CHAT_H

#include <QMainWindow>
#include "client.h"

namespace Ui {
class Chat;
}

class Chat : public QMainWindow
{
    Q_OBJECT

public:
    explicit Chat(QWidget *parent = nullptr);
    ~Chat();
private slots:
    void connectClicked();
    void disconnectClicked();
    void sendClicked();

    void clientDisconnected();

    void showMessage(const QString& message);
private:
    Ui::Chat *ui;
    Client* client;
};

#endif // CHAT_H
