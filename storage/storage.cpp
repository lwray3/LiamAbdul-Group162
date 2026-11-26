#include "storage.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSaveFile>

JsonFileStorage::JsonFileStorage(const QString& filePath)
    : filePath(filePath)
{
}

JsonFileStorage::LoadStatus JsonFileStorage::load(
    QJsonObject& data,
    QString& error) const
{
    error.clear();

    QFile file(filePath);

    if (!file.exists()) {
        error = "Save file does not exist.";
        return LoadStatus::NotFound;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        error = file.errorString();
        return LoadStatus::Error;
    }

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(
        file.readAll(), &parseError
        );

    if (parseError.error != QJsonParseError::NoError ||
        !document.isObject()) {
        error = "Invalid JSON save file.";
        return LoadStatus::Error;
    }

    QJsonObject loaded = document.object();

    if (!loaded.value("schema_version").isDouble() ||
        loaded.value("schema_version").toInt() != 1) {
        error = "Unsupported save format version.";
        return LoadStatus::Error;
    }

    data = loaded;
    return LoadStatus::Success;
}

bool JsonFileStorage::save(
    const QJsonObject& data,
    QString& error) const
{
    error.clear();

    QFileInfo info(filePath);

    if (!QDir().mkpath(info.absolutePath())) {
        error = "Cannot create save directory.";
        return false;
    }

    QSaveFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        error = file.errorString();
        return false;
    }

    QJsonDocument document(data);
    QByteArray json = document.toJson(QJsonDocument::Indented);

    if (file.write(json) != json.size()) {
        error = file.errorString();
        file.cancelWriting();
        return false;
    }

    if (!file.commit()) {
        error = file.errorString();
        return false;
    }

    return true;
}
