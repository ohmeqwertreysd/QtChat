#ifndef CHAT_H
#define CHAT_H

#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QThread>
#include "filewidgetitem.h"
#include "../src/jsonbuild.h"
#include "../src/jsonparse.h"
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
signals:
    void connectServer(const QHostAddress& addr, const quint16& port);
    void disconnectServer();
    void loginUser(const QString&, const QString&);
    void registerUser(const QString&, const QString&);
    void downloadFile(const QString&, const QString&, const QString&);
    void sendFile(const QString&, const QString&);
    void sendMessage(const QString&, const QString&);
private slots:
    void connectClicked();
    void disconnectClicked();
    void sendClicked();
    void loginClicked();
    void registrationClicked();
    void downloadClicked();
    void uploadClicked();

    void sendPressed();
    void sendReleased();

    void clientConnected();
    void clientDisconnected();
    void errorOccured(QAbstractSocket::SocketError socketError);
    void loginFailed(const QString& error);

    void successAuthorization(const QJsonObject& json);

    void showNewMessage(const JsonMessage& json);
    void showNewFile(const JsonFile& json);
    void updateListOfMessages(JsonMessage json);
    void updateListOfOnlineUsers(JsonUser json);
    void updateListOfFiles(JsonFile json);
private:
    QThread clientThread;
    Ui::Chat *ui_chat;
    bool isConnected;
    bool isAuthorized;
};

#endif // CHAT_H
