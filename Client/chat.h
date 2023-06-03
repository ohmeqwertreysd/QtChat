#ifndef CHAT_H
#define CHAT_H

#include <QMainWindow>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
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

    void changeStateFields();
private slots:
    void connectClicked();
    void disconnectClicked();
    void sendClicked();
    void sendFileClicked();

    void clientDisconnected();
    void errorOccured(QAbstractSocket::SocketError socketError);

    void showMessage(const QJsonObject& json);
    void setChatCursorToEnd();
private:
    Ui::Chat *ui;
    Client* client;
};

#endif // CHAT_H
