#include "utils.h"
#include <QString>
#include <QChar>
#include <QStringList>
QByteArray toHex(QByteArray& bytes)
{
    QByteArray result;
    bytes = bytes.toHex();
    for(int i = 0;i<bytes.length();i+=2)
    {
        result += bytes.mid(i,2) + " ";
    }
    return result;
}

QString toAscii(QString& hexStr)
{
    QStringList parts = hexStr.split(' ',Qt::SkipEmptyParts);
    QByteArray result;
    for(const QString& part : parts)
    {
        if(part == "0a")
        {
            result.append('\n');
            continue;
        }
        bool ok = false;
        char c = static_cast<char>(part.toInt(&ok,16));
        if(ok)
        {
            result.append(c);
        }
        else
        {
            result.append('?');
        }
    }
    return QString::fromUtf8(result);
}
