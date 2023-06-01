
#ifndef USER_H
#define USER_H


#include <QString>
#include <QTcpSocket>

class User
{
public:
    User();
    ~User();

    void setUserName(const QString& name);
    void setSocket(QTcpSocket* socket);

    QTcpSocket* getSocket();
    QString getUserName();
private:
    QString _userName;
    QTcpSocket* _socket;
};

#endif // USER_H
