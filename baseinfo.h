#ifndef BASEINFO_H
#define BASEINFO_H

#include <QString>
#include <QTextStream>

class BaseInfo
{
private:
    QString ss;
public:
    BaseInfo( QString text = "" )
    {
        ss = text;
    }
    template<typename T>
    BaseInfo& operator<<(T &el)
    {
        QTextStream(ss) << el;
        return this;
    }
    QString get()
    {
        return ss;
    }

signals:
public slots:
};

class Error : public BaseInfo
{
public:
    Error( QString text = ""  ) : BaseInfo(text)
    {
    }
};

// TODO:
// USBError

#endif // BASEINFO_H
