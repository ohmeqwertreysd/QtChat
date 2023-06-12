#include "filewidgetitem.h"

FileWidgetItem::FileWidgetItem(QWidget* parent)
    : QWidget(parent)
{
    this->filename_displayed = new QLabel;
    this->filelength = new QLabel;
    this->datetime = new QLabel;
    this->icon = new QLabel;
    this->textLayout = new QVBoxLayout;
    this->textLayout->addWidget(this->filename_displayed);
    this->textLayout->addWidget(this->filelength);
    this->textLayout->addWidget(this->datetime);
    this->textLayout->setSpacing(0);
    this->formLayout = new QHBoxLayout;
    this->formLayout->layout()->setContentsMargins(1,1,1,1);
    this->formLayout->addWidget(this->icon, 0);
    this->formLayout->addLayout(textLayout, 1);
    this->setLayout(formLayout);


    this->filename_displayed->setStyleSheet("font: 10pt \"Open Sans\";");
    this->filelength->setStyleSheet("font: 9pt \"Open Sans\"; color: white");
    this->datetime->setStyleSheet("font: 9pt \"Open Sans\"; color: white;");
}

void FileWidgetItem::setFileName(const QString& text, int stringWidth)
{

    QFontMetrics metrics(this->filename_displayed->font());
    this->filename_displayed->setText(metrics.elidedText(text, Qt::ElideRight, stringWidth - 35));
    this->filename_displayed->setToolTip(text);
    this->filename = text;
}

void FileWidgetItem::setFileLength(qint64 num)
{
    QString unit[] = {"B", "KB", "MB", "GB"};
    quint8 un = 0;
    double size = num;
    while(static_cast<qint64>(size) / 1024 && un < sizeof(unit) / sizeof(QString))
    {
        size /= 1024.0;
        ++un;
    }
    this->filelength->setText(QString::number(size, 'f', 2) + " " + unit[un]);
}

void FileWidgetItem::setDateTime(const QString& text)
{
    this->datetime->setText(text);
}

QString FileWidgetItem::getFileName()
{
    return this->filename;
}

void FileWidgetItem::setIcon(const QString& iconPath)
{
    QPixmap pix(iconPath);
    this->icon->setPixmap(pix.scaled(32,32, Qt::IgnoreAspectRatio));
}
