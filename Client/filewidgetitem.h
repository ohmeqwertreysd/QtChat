
#ifndef FILEWIDGETITEM_H
#define FILEWIDGETITEM_H


#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QFontMetrics>

class FileWidgetItem : public QWidget
{
    Q_OBJECT
public:
    explicit FileWidgetItem(QWidget *parent = nullptr);
    void setFileName(const QString& text, int stringWidth);
    void setFileLength(qint64 num);
    void setDateTime(const QString& text);
    void setIcon(const QString& iconPath);

    QString getFileName();
private:
    QString filename;
    QLabel* filename_displayed;
    QLabel* filelength;
    QLabel* datetime;
    QLabel* icon;
    QVBoxLayout* textLayout;
    QHBoxLayout* formLayout;
};

#endif // FILEWIDGETITEM_H
