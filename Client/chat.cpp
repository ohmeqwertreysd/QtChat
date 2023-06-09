#include "chat.h"
#include "ui_chat.h"

Chat::Chat(QWidget *parent) :
    QMainWindow(parent),
    ui_chat(new Ui::Chat),
    client(new Client(this))
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
    connect(client, SIGNAL(messageReceived(QJsonObject)), this, SLOT(showNewMessage(QJsonObject)));
    connect(client, SIGNAL(fileReceived(QJsonObject)), this, SLOT(showNewFile(QJsonObject)));
    connect(client, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(errorOccured(QAbstractSocket::SocketError)));
    connect(client, SIGNAL(loginFailed(QString)), this, SLOT(loginFailed(QString)));
    connect(client, SIGNAL(successAuthorizated(QJsonObject)), this, SLOT(successAuthorization(QJsonObject)));
    connect(client, SIGNAL(listOfUsersReceived(QJsonObject)), this, SLOT(updateListOfOnlineUsers(QJsonObject)));
    connect(client, SIGNAL(listOfFilesReceived(QJsonObject)), this, SLOT(updateListOfFiles(QJsonObject)));
    connect(client, SIGNAL(fileProgressChanged(int)), ui_chat->progressBar, SLOT(setValue(int)));

    connect(client, SIGNAL(fileProgressStart()), this, SLOT(fileProgressStarted()));
    connect(client, SIGNAL(fileProgressEnd()), this, SLOT(fileProgressEnded()));

    connect(this, SIGNAL(downloadFile(QString,QString,QString)), client, SLOT(downloadFile(QString,QString,QString)));
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
    QListWidget * widgets = item->listWidget();
    QWidget* widget = widgets->itemWidget(item);
    QString filename_original = dynamic_cast<FileWidgetItem*>(widgets->itemWidget(item))->getFileName();
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
    QJsonArray json_array = json.value("Data").toArray();
    for(const QJsonValue& val : json_array)
    {
        QJsonObject json_obj = val.toObject();
        if(json_obj.contains("Messages"))
        {
            updateListOfMessages(json_obj);
        }
        else if(json_obj.contains("Files"))
        {
            updateListOfFiles(json_obj);
        }
    }
}

void Chat::showNewMessage(const QJsonObject& json)
{
    QString message = QString("%1 [%2]: %3").
                      arg(json.value("DateTime").toString(),
                          json.value("Username").toString(),
                          json.value("Message").toString());
    ui_chat->chatBrowser->append(message);
}

void Chat::showNewFile(const QJsonObject& json)
{
    FileWidgetItem* item = new FileWidgetItem(this);
    item->setFileName(json.value("Filename").toString(), ui_chat->filesListWidget->batchSize());
    item->setFileLength(json.value("FileLength").toInteger());
    item->setDateTime(json.value("DateTime").toString());
    item->setIcon(":/icon/Resource/file_notselected.png");
    QListWidgetItem* widgetItem = new QListWidgetItem(ui_chat->filesListWidget);
    widgetItem->setSizeHint(item->sizeHint());
    ui_chat->filesListWidget->addItem(widgetItem);
    ui_chat->filesListWidget->setItemWidget(widgetItem, item);
}

void Chat::updateListOfMessages(const QJsonObject& json)
{
    QJsonArray json_array = json.value("Messages").toArray();
    for(const QJsonValue& val : json_array)
    {
        if(val.isObject())
        {
            showNewMessage(val.toObject());
        }
    }
}

void Chat::updateListOfOnlineUsers(const QJsonObject& json)
{
    ui_chat->usersList->clear();
    QJsonArray json_array = json.value("Users").toArray();
    for(const QJsonValue& val : json_array)
    {
        ui_chat->usersList->addItem(val.toString());
    }
}

void Chat::updateListOfFiles(const QJsonObject& json)
{
    QJsonArray json_array = json.value("Files").toArray();
    for(const QJsonValue& val : json_array)
    {
        if(val.isObject())
        {
            showNewFile(val.toObject());
        }
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
