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
    connect(ui->sendFileButton, SIGNAL(clicked()), this, SLOT(sendFileClicked()));

    connect(client, SIGNAL(messageReceived(QJsonObject)), this, SLOT(showMessage(QJsonObject)));
    connect(client, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    connect(client, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(errorOccured(QAbstractSocket::SocketError)));
    connect(ui->chatBrowser, SIGNAL(textChanged()), this, SLOT(setChatCursorToEnd()));
}

Chat::~Chat()
{
    delete ui;
}

void Chat::changeStateFields()
{
    ui->disconnectButton->setEnabled(!ui->disconnectButton->isEnabled());
    ui->connectButton->setEnabled(!ui->connectButton->isEnabled());
    ui->nameString->setEnabled(!ui->nameString->isEnabled());
    ui->addrString->setEnabled(!ui->addrString->isEnabled());
    ui->portString->setEnabled(!ui->portString->isEnabled());
}

void Chat::connectClicked()
{
    client->connectServer(ui->nameString->text(), QHostAddress(ui->addrString->text()), ui->portString->text().toUInt());
    changeStateFields();
}

void Chat::disconnectClicked()
{
    client->disconnectServer();
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

void Chat::sendFileClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File");
}

void Chat::clientDisconnected()
{
    changeStateFields();
    QString message = QString("<p align=\"left\" style=\" background-color:#ccc; margin-top:5px; margin-bottom:0px; margin-left:0px; margin-right:200px; -qt-block-indent:0; text-indent:0px;\"><span style=\"font-size:10pt;\">%1</span></p>")
                  .arg("You have been disconnected from the chat.");
    QString dateTimeInfo = QString("<p align=\"left\" style=\" background-color:#ccc; margin-top:0px; margin-bottom:5px; margin-left:0px; margin-right:200px; -qt-block-indent:0; text-indent:0px;\"><span style=\"font-size:8pt;\">%1 %2</span></p>")
                       .arg(QDate::currentDate().toString("dd.MM.yy"), QTime::currentTime().toString("hh:mm:ss"));
    ui->chatBrowser->setText(ui->chatBrowser->toHtml() + message + dateTimeInfo);
}

void Chat::errorOccured(QAbstractSocket::SocketError socketError)
{
    changeStateFields();
}

void Chat::showMessage(const QJsonObject& json)
{

    QString message;
    QString dateTimeInfo;
    ui->chatBrowser->append("<p></p>");
    if(json.value("Name").toString() == ui->nameString->text())
    {
        message = QString("<p align=\"right\" style=\"background-color:#fc0; margin-top:5px; margin-bottom:0px; margin-left:200px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\"font-size:10pt;\">%1</span></p>")
                      .arg(json.value("Message").toString());
        dateTimeInfo = QString("<p align=\"right\" style=\"background-color:#fc0; margin-top:0px; margin-bottom:5px; margin-left:200px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\"font-size:8pt;\">%1 %2</span></p>")
                           .arg(json.value("Date").toString(), json.value("Time").toString());
    }
    else
    {
        message = QString("<p align=\"left\" style=\" background-color:#ccc; margin-top:5px; margin-bottom:0px; margin-left:0px; margin-right:200px; -qt-block-indent:0; text-indent:0px;\"><span style=\"font-size:10pt;\">[%1]: %2</span></p>")
                      .arg(json.value("Name").toString(), json.value("Message").toString());
        dateTimeInfo = QString("<p align=\"left\" style=\" background-color:#ccc; margin-top:0px; margin-bottom:5px; margin-left:0px; margin-right:200px; -qt-block-indent:0; text-indent:0px;\"><span style=\"font-size:8pt;\">%1 %2</span></p>")
                           .arg(json.value("Date").toString(), json.value("Time").toString());
    }
    ui->chatBrowser->setText(ui->chatBrowser->toHtml() + message + dateTimeInfo);
}

void Chat::setChatCursorToEnd()
{
    QTextCursor cursor = ui->chatBrowser->textCursor();
    cursor.movePosition(QTextCursor::EndOfLine);
    ui->chatBrowser->setTextCursor(cursor);
}
