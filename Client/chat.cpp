#include "chat.h"
#include "ui_chat.h"

Chat::Chat(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Chat),
    client(new Client(this))
{
    ui->setupUi(this);

    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(connectClicked()));
    connect(ui->disconnectButton, SIGNAL(clicked()), this, SLOT(disconnectClicked()));
    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(sendClicked()));

    connect(client, SIGNAL(messageReceived(QString)), this, SLOT(showMessage(QString)));
    connect(client, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
}

Chat::~Chat()
{
    delete ui;
}

void Chat::showMessage(const QString& message)
{
    ui->chatBrowser->append(message);
}

void Chat::connectClicked()
{
    client->connectServer(ui->nameString->text(), QHostAddress(ui->addrString->text()), ui->portString->text().toUInt());
    ui->disconnectButton->setEnabled(true);
    ui->connectButton->setEnabled(false);
}

void Chat::disconnectClicked()
{
    client->disconnectServer();
    ui->disconnectButton->setEnabled(false);
    ui->connectButton->setEnabled(true);
}

void Chat::sendClicked()
{
    if(ui->messageString->text().isEmpty())
    {
        return;
    }
    client->sendMessage(ui->messageString->text());
    ui->messageString->clear();
}

void Chat::clientDisconnected()
{
    ui->chatBrowser->append("You have been disconnected from the chat.");
}
