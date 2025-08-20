#ifndef LOGGER_H
#define LOGGER_H

#include <QDebug>
#include <QString>
#include <QProcessEnvironment>
#include <QLoggingCategory>

class Logger
{
public:
    static bool isLoggingEnabled() {
        static bool checked = false;
        static bool enabled = false;
        
        if (!checked) {
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            QString logValue = env.value("PLAYER_LOG", "0");
            enabled = (logValue == "1");
            checked = true;
        }
        
        return enabled;
    }
    
    static QDebug debug() {
        if (isLoggingEnabled()) {
            return qDebug();
        } else {
            static QString dummyString;
            return QDebug(&dummyString);
        }
    }
};

// Macro for convenient usage  
#define LOG_DEBUG(...) do { if (Logger::isLoggingEnabled()) { qDebug() << __VA_ARGS__; } } while(0)

#endif // LOGGER_H