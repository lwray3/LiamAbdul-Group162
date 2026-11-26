#include <QApplication>
#include "ui/mainwindow.h"

#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QJsonObject>
#include <QMessageBox>
#include <QDebug>
#include "storage/storage.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationName("MirageGroup162");
    a.setApplicationName("MirageCasino");

    QString saveDirectory =
        QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation
            );

    if (saveDirectory.isEmpty()) {
        QMessageBox::critical(
            nullptr, "Storage Error",
            "Cannot determine the save directory."
            );
        return 1;
    }

    QString savePath =
        QDir(saveDirectory).filePath("mirage_save.json");

    JsonFileStorage storage(savePath);

    QJsonObject gameData;
    QString error;

    // Try loading the player's existing save
    auto status = storage.load(gameData, error);

    if (status == JsonFileStorage::LoadStatus::Success) {

        qDebug() << "Existing save loaded successfully.";

    }
    else if (status == JsonFileStorage::LoadStatus::NotFound) {

        qDebug() << "No save found. Loading default data.";

        JsonFileStorage defaultStorage(
            ":/data/default.json"
            );

        auto defaultStatus =
            defaultStorage.load(gameData, error);

        if (defaultStatus !=
            JsonFileStorage::LoadStatus::Success) {

            QMessageBox::critical(
                nullptr, "Storage Error",
                "Cannot load default data: " + error
                );
            return 1;
        }

        // Create the player's initial save file
        if (!storage.save(gameData, error)) {
            QMessageBox::critical(
                nullptr, "Storage Error",
                "Cannot create save file: " + error
                );
            return 1;
        }

        qDebug() << "Initial save created.";
    }
    else {
        // Existing file could not be loaded.
        // Do not overwrite potentially recoverable data.
        QMessageBox::critical(
            nullptr, "Storage Error",
            "Cannot load the existing save file: " + error
            );
        return 1;
    }

    qDebug() << "Save location:" << savePath;

    MainWindow w;
    w.show();

    return a.exec();
}
