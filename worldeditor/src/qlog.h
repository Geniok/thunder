#ifndef QTLOG_H
#define QTLOG_H

#include <QObject>

#include <log.h>

class QLog : public QObject, public ILogHandler {
    Q_OBJECT

public:
    void                setRecord       (Log::LogTypes type, const char *record) {
        emit postRecord(type, QString(record));
	}

signals:
    void                postRecord      (uint8_t type, const QString &str);

};

#endif // QTLOG_H
