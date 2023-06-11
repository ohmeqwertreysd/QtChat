#include "chat.h"
#include "ui_chat.h"

Chat::Chat(QWidget *parent) :
    QMainWindow(parent),
    ui_chat(new Ui::Chat),
    client(new Client())
{
    ui_chat->setupUi(this);
    ui_chat->progressBar->setVisible(false);
    connect(ui_chat->connectButton, SIGNAL(clicked()), this, SLOT(connectClicked()));
    connect(ui_chat->disconnectButton, SIGNAL(clicked()), this, SLOT(disconnectClicked()));
    connect(ui_chat->sendButton, SIGNAL(clicked()), this, SLOT(sendClicked()));
    connect(ui_chat->loginButton, SIGNAL(clicked()), this, SLOT(loginClicked()));
    connect(ui_chat->registrationButton, SIGNAL(clicked()), this, SLOT(registrationClicked()));
    connect(ui_chat->chatBrowser, SIGNAL(textChanged()), this, SLOT(setChatCursorToEnd()));
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

    connect(client, SIGNAL(fileProgressChanged(int)), ui_chat->progressBar, SLOT(setValue(int)));
    connect(client, SIGNAL(fileProgressStart()), this, SLOT(fileProgressStarted()));
    connect(client, SIGNAL(fileProgressEnd()), this, SLOT(fileProgressEnded()));

    connect(this, SIGNAL(downloadFile(QString,QString,QString)), client, SLOT(downloadRequestFile(QString,QString,QString)));
    connect(this, SIGNAL(sendFile(QString,QString)), client, SLOT(sendFile(QString,QString)));
}

Chat::~Chat()
{
    delete ui_chat;
}

void Chat::connectClicked()
{
    if(client->isConnected())
    {
        return;
    }

    client->connectServer(QHostAddress(ui_chat->addrString->text()), ui_chat->portString->text().toUInt());
}

void Chat::disconnectClicked()
{
    if(!client->isConnected())
    {
        return;
    }

    client->disconnectServer();
}

void Chat::sendClicked()
{
    if(ui_chat->messageString->text().isEmpty() || !client->isConnected() || !client->isAuth())
    {
        return;
    }
    client->sendMessage(ui_chat->usernameString->text(), ui_chat->messageString->text());
    ui_chat->messageString->clear();
}

void Chat::loginClicked()
{
    if(client->isConnected() && !client->isAuth())
    {
        qDebug() << "login";
        client->loginUser(ui_chat->usernameString->text(), ui_chat->passwordString->text());
    }
}

void Chat::registrationClicked()
{
    if(client->isConnected() && !client->isAuth())
    {
        client->registerUser(ui_chat->usernameString->text(), ui_chat->passwordString->text());
    }
}

void Chat::downloadClicked()
{
    if(!client->isConnected() || !client->isAuth())
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
    if(!client->isConnected() || !client->isAuth())
    {
        return;
    }
    QString filename = QFileDialog::getOpenFileName(this, "Choose file");
    emit sendFile(ui_chat->usernameString->text(), filename);
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
    QMessageBox dialog(QMessageBox::Information, "Log in", "Sign in or Register");
    dialog.exec();
}

void Chat::clientDisconnected()
{
    ui_chat->usersList->clear();
    ui_chat->filesListWidget->clear();
    ui_chat->chatBrowser->clear();
}

void Chat::errorOccured(QAbstractSocket::SocketError socketError)
{

}

void Chat::loginFailed(const QString& error)
{
    QMessageBox dialog(QMessageBox::Warning, "Log in", error);
    dialog.exec();
}

void Chat::successAuthorization(const QJsonObject& json)
{
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
    FileWidgetItem* item = new FileWidgetItem(this);
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
    qDebug() << "User list";
    while(json.next())
    {
        qDebug() << json.getUserName();
        ui_chat->usersList->addItem(json.getUserName());
    }
}

void Chat::updateListOfFiles(JsonFile json)
{
    while(json.next())
    {
        showNewFile(json);
    }
}

void Chat::fileProgressStarted()
{
    ui_chat->progressBar->setVisible(true);
}

void Chat::fileProgressEnded()
{
    ui_chat->progressBar->setVisible(false);
    ui_chat->progressBar->setValue(0);
}

void Chat::setChatCursorToEnd()
{
    QTextCursor cursor = ui_chat->chatBrowser->textCursor();
    cursor.movePosition(QTextCursor::EndOfLine);
    ui_chat->chatBrowser->setTextCursor(cursor);
}
