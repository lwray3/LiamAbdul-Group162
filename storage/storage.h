
#ifndef STORAGE_H
#define STORAGE_H

#include <QJsonObject>
#include <QString>

class JsonFileStorage
{
public:
    enum class LoadStatus {
        Success,
        NotFound,
        Error
    };

    explicit JsonFileStorage(const QString& filePath);

    LoadStatus load(QJsonObject& data, QString& error) const;

    bool save(const QJsonObject& data, QString& error) const;

private:
    QString filePath;
};

#endif
