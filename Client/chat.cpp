#include "chat.h"
#include "ui_chat.h"

Chat::Chat(QWidget *parent) :
    QMainWindow(parent),
    ui_chat(new Ui::Chat),
    isConnected(false),
    isAuthorized(false)
{
    Client* client = new Client;
    client->moveToThread(&clientThread);
    ui_chat->setupUi(this);
    ui_chat->progressBar->setVisible(false);
    connect(ui_chat->connectButton, SIGNAL(clicked()), this, SLOT(connectClicked()));
    connect(ui_chat->disconnectButton, SIGNAL(clicked()), this, SLOT(disconnectClicked()));
    connect(ui_chat->sendButton, SIGNAL(clicked()), this, SLOT(sendClicked()));
    connect(ui_chat->loginButton, SIGNAL(clicked()), this, SLOT(loginClicked()));
    connect(ui_chat->registrationButton, SIGNAL(clicked()), this, SLOT(registrationClicked()));
    connect(ui_chat->downloadButton, SIGNAL(clicked()), this, SLOT(downloadClicked()));
    connect(ui_chat->uploadButton, SIGNAL(clicked()), this, SLOT(uploadClicked()));

    connect(ui_chat->sendButton, SIGNAL(pressed()), this, SLOT(sendPressed()));
    connect(ui_chat->sendButton, SIGNAL(released()), this, SLOT(sendReleased()));

    connect(client, SIGNAL(connected()), this, SLOT(clientConnected()));
    connect(client, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    connect(client, SIGNAL(messageReceived(JsonMessage)), this, SLOT(showNewMessage(JsonMessage)));
    connect(client, SIGNAL(fileReceived(JsonFile)), this, SLOT(showNewFile(JsonFile)));
    connect(client, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(errorOccured(QAbstractSocket::SocketError)));
    connect(client, SIGNAL(loginFailed(QString)), this, SLOT(loginFailed(QString)));
    connect(client, SIGNAL(successAuthorizated(QJsonObject)), this, SLOT(successAuthorization(QJsonObject)));
    connect(client, SIGNAL(listOfUsersReceived(JsonUser)), this, SLOT(updateListOfOnlineUsers(JsonUser)));
    connect(client, SIGNAL(listOfFilesReceived(JsonFile)), this, SLOT(updateListOfFiles(JsonFile)));

    connect(this, SIGNAL(connectServer(QHostAddress,quint16)), client, SLOT(connectServer(QHostAddress,quint16)));
    connect(this, SIGNAL(disconnectServer()), client, SLOT(disconnectServer()));
    connect(this, SIGNAL(loginUser(QString,QString)), client, SLOT(loginUser(QString,QString)));
    connect(this, SIGNAL(registerUser(QString,QString)), client, SLOT(registerUser(QString,QString)));
    connect(this, SIGNAL(downloadFile(QString,QString,QString)), client, SLOT(downloadRequestFile(QString,QString,QString)));
    connect(this, SIGNAL(sendFile(QString,QString)), client, SLOT(sendFile(QString,QString)));
    connect(this, SIGNAL(sendMessage(QString,QString)), client, SLOT(sendMessage(QString,QString)));
    clientThread.start();
}

Chat::~Chat()
{
    clientThread.quit();
    clientThread.wait();
    delete ui_chat;
}

void Chat::connectClicked()
{
    if(this->isConnected)
    {
        QMessageBox dialog(QMessageBox::Warning, "Connection", "You are already connected to the server");
        dialog.exec();
    }
    else
    {
        emit connectServer(QHostAddress(ui_chat->addrString->text()), ui_chat->portString->text().toUInt());
    }
}

void Chat::disconnectClicked()
{
    if(!this->isConnected)
    {
        QMessageBox dialog(QMessageBox::Warning, "Connection", "You are not connected to the server");
        dialog.exec();
    }
    else
    {
        emit disconnectServer();
    }
}

void Chat::sendClicked()
{
    if(!ui_chat->messageString->text().isEmpty() && this->isConnected && this->isAuthorized)
    {
        emit sendMessage(ui_chat->usernameString->text(), ui_chat->messageString->text());
        ui_chat->messageString->clear();
    }
}

void Chat::loginClicked()
{
    if(this->isConnected && !this->isAuthorized)
    {
        emit loginUser(ui_chat->usernameString->text(), ui_chat->passwordString->text());
    }
    else
    {
        QMessageBox dialog(QMessageBox::Warning, "Connection", "You are not connected to the server");
        dialog.exec();
    }
}

void Chat::registrationClicked()
{
    if(this->isConnected && !this->isAuthorized)
    {
        emit registerUser(ui_chat->usernameString->text(), ui_chat->passwordString->text());
    }
    else
    {
        QMessageBox dialog(QMessageBox::Warning, "Connection", "You are not connected to the server");
        dialog.exec();
    }
}

void Chat::downloadClicked()
{
    if(!this->isConnected || !this->isAuthorized)
    {
        return;
    }
    QListWidgetItem* item = ui_chat->filesListWidget->currentItem();
    if(item == nullptr)
    {
        return;
    }
    QListWidget * listWidget = item->listWidget();
    QString filename_original = dynamic_cast<FileWidgetItem*>(listWidget->itemWidget(item))->getFileName();
    QString filename = QFileDialog::getSaveFileName(this, "Save File", filename_original);
    emit downloadFile(ui_chat->usernameString->text(), filename, filename_original);
}

void Chat::uploadClicked()
{
    if(this->isConnected && this->isAuthorized)
    {
        QString filename = QFileDialog::getOpenFileName(this, "Choose file");
        emit sendFile(ui_chat->usernameString->text(), filename);
    }
}

void Chat::sendPressed()
{
    QIcon icon(":/icon/Resource/send_clicked.png");
    ui_chat->sendButton->setIcon(icon);
}

void Chat::sendReleased()
{
    QIcon icon(":/icon/Resource/send_notclicked.png");
    ui_chat->sendButton->setIcon(icon);
}

void Chat::clientConnected()
{
    this->isConnected = true;
    QMessageBox dialog(QMessageBox::Information, "Log in", "Sign in or Register");
    dialog.exec();
}

void Chat::clientDisconnected()
{
    this->isConnected = false;
    this->isAuthorized = false;
    ui_chat->usersList->clear();
    ui_chat->filesListWidget->clear();
    ui_chat->chatBrowser->clear();
}

void Chat::errorOccured(QAbstractSocket::SocketError socketError)
{

}

void Chat::loginFailed(const QString& error)
{
    this->isAuthorized = false;
    QMessageBox dialog(QMessageBox::Warning, "Log in", error);
    dialog.exec();
}

void Chat::successAuthorization(const QJsonObject& json)
{
    this->isAuthorized = true;
    updateListOfMessages(JsonMessage(json));
    updateListOfFiles(JsonFile(json));
}

void Chat::showNewMessage(const JsonMessage& json)
{
    QString message = QString("%1 [%2]: %3").
                      arg(json.getDateTime(),
                          json.getUserName(),
                          json.getMessage());
    ui_chat->chatBrowser->append(message);
}

void Chat::showNewFile(const JsonFile& json)
{
    FileWidgetItem* item = new FileWidgetItem();
    item->setFileName(json.getFileName(), ui_chat->filesListWidget->batchSize());
    item->setFileLength(json.getFileLength());
    item->setDateTime(json.getDateTime());
    item->setIcon(":/icon/Resource/file_notselected.png");
    QListWidgetItem* widgetItem = new QListWidgetItem(ui_chat->filesListWidget);
    widgetItem->setSizeHint(item->sizeHint());
    ui_chat->filesListWidget->addItem(widgetItem);
    ui_chat->filesListWidget->setItemWidget(widgetItem, item);
}

void Chat::updateListOfMessages(JsonMessage json)
{
    while(json.next())
    {
        showNewMessage(json);
    }
}

void Chat::updateListOfOnlineUsers(JsonUser json)
{
    ui_chat->usersList->clear();
    while(json.next())
    {
        qDebug() << json.getUserName();
        ui_chat->usersList->addItem(json.getUserName());
    }
}

void Chat::updateListOfFiles(JsonFile json)
{
    qDebug() << "updateListOfFiles";
    while(json.next())
    {
        showNewFile(json);
    }
}
