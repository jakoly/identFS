#include "addfiles.h"
#include "ui_addfiles.h"
#include "mainwindow.h"
#include <QListWidgetItem>
#include <QFont>
#include <QIcon>
#include <iostream>
#include <sstream>
#include <QSettings>
#include <QDebug>
#include <QTextStream>
#include <QUuid>
#include <QString>
#include <fstream>
#include <string>
#include <QCryptographicHash>
#include <QString>
#include <QByteArray>
#include <QMessageBox>

namespace fs = std::filesystem;
using namespace std;


addFiles::addFiles(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::addFiles)
{
    ui->setupUi(this);

    connect(ui->pushButtonAddFiles, &QPushButton::clicked, this, &addFiles::selectFiles);
    connect(ui->pushButtonOk, &QPushButton::clicked, this, &addFiles::onOkClicked);
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &addFiles::onCancelClicked);




    //open DB
    db = nullptr;
    int rc = sqlite3_open("database/identfs.db", &db);
    if(rc) {
        std::cerr << "Database can't be opened: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Database opened successfully!" << std::endl;
    }

    // --- CODE ---
    //get drive letter
    std::ifstream in("settings/stdPath.txt");
    std::string tempString;
    std::getline(in, tempString);
    installPath = QString::fromStdString(tempString).trimmed();

}

addFiles::~addFiles()
{
    delete ui;
}

float addFiles::getFileSize(const std::string &filename)
{
    std::ifstream in(filename.c_str(), std::ios::binary);
    if (!in) return 0; // Datei konnte nicht geöffnet werden

    in.seekg(0, std::ios::end);        // ans Ende der Datei springen
    std::streampos size = in.tellg();  // Position = Dateigröße in Bytes
    return static_cast<float>(size / 1024); // in KB umrechnen
}

std::string addFiles::getFileNameFromPath(const std::string& filePath) {
    std::filesystem::path p(filePath);
    return p.filename().string(); // gibt nur den Dateinamen zurück
}

void addFiles::selectFiles()
{
    files = QFileDialog::getOpenFileNames(
        this,
        "Dateien auswählen",
        "",  // Startordner
        "Alle Dateien (*);;Bilder (*.png *.jpg *.bmp)"
        );
    if (!files.isEmpty()) {
        for (const QString &filePath : files) {
            QString name = QString::fromStdString(getFileNameFromPath(filePath.toStdString()));
            QListWidgetItem *item = new QListWidgetItem(QIcon(":/icons/icons/unkown_file.png"), name);
            ui->listWidgetFiles->addItem(item);
            qDebug() << "Ausgewählte Datei:" << filePath;
        }
    }
}

void addFiles::onOkClicked() {
    qDebug() << "OK gedrueckt!";

    if (!files.isEmpty()) {
        for (const QString &filePath : files) {
            // --- UUID erzeugen ---
            QUuid uuid = QUuid::createUuid();
            QString uuidText = uuid.toString();
            uuidText.remove(QRegularExpression("[{}]"));
            QString name = QString::fromStdString(getFileNameFromPath(filePath.toStdString()));
            int fileSize = getFileSize(filePath.toStdString());

            // --- Hash erzeugen ---
            std::ifstream in(filePath.toStdString(), std::ios::in | std::ios::binary);
            if (!in.is_open()) {
                std::cerr << "Datei konnte nicht geöffnet werden\n";
                continue; // nächste Datei
            }
            std::ostringstream contents;
            contents << in.rdbuf();
            std::string fileContents = contents.str();

            std::string fileHashStd = hashText(fileContents);
            QString fileHash = QString::fromStdString(fileHashStd);
            QByteArray hashUtf8 = fileHash.toUtf8();

            qDebug() << "File-Hash: " << fileHash;

            // --- Prüfen, ob Datei schon in DB ist ---
            sqlite3_stmt* checkStmt;
            const char* sqlCheck = "SELECT COUNT(*) FROM files WHERE file_hash = ?;";
            int rcCheck = sqlite3_prepare_v2(db, sqlCheck, -1, &checkStmt, nullptr);
            if (rcCheck != SQLITE_OK) {
                std::cerr << "Error preparing check statement: " << sqlite3_errmsg(db) << std::endl;
                continue; // nächste Datei
            }

            sqlite3_bind_text(checkStmt, 1, hashUtf8.constData(), -1, SQLITE_TRANSIENT);

            rcCheck = sqlite3_step(checkStmt);
            bool alreadyExists = false;
            if (rcCheck == SQLITE_ROW) {
                int count = sqlite3_column_int(checkStmt, 0);
                if (count > 0) {
                    alreadyExists = true;
                }
            }

            sqlite3_finalize(checkStmt);

            if (alreadyExists) {
                qDebug() << "Datei wurde bereits hinzugefügt: " << filePath;
                QMessageBox::information(
                    this,                                // Parent-Widget, meist "this"
                    "Datei bereits hinzugefügt",          // Titel des Fensters
                    QString("Die Datei '%1' wurde bereits hinzugefügt.").arg(filePath)  // Nachricht
                    );
                continue;
            }

            // --- UUID in eigenem ADS speichern ---
            QString uuidStream = filePath + ":IdentFS_UUID";
            QFile file(uuidStream);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << "UUID=" << uuidText;
                file.close();
                qDebug() << "UUID geschrieben:" << uuidText;
            } else {
                qDebug() << "Fehler beim Öffnen des ADS für" << filePath;
            }

            // --- Add Copy to Vault ---
            QString default_path = installPath + "/vault/00000000-0000-0000-0000-000000000000";
            std::filesystem::create_directories(default_path.toStdString());

            std::filesystem::path destPath = std::filesystem::path(default_path.toStdString()) / std::filesystem::path(filePath.toStdString()).filename();

            std::filesystem::copy_file(filePath.toStdString(),
                                       destPath,
                                       std::filesystem::copy_options::overwrite_existing);

            QString currentVaultPath = default_path;

            // --- Insert in DB ---
            sqlite3_stmt* stmt;
            const char* sqlInsert = "INSERT INTO files (file_uuid, last_path, size, active, name, file_hash, current_vault_path) VALUES (?, ?, ?, 1, ?, ?, ?);";
            int rc = sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, nullptr);
            if (rc != SQLITE_OK) {
                std::cerr << "Error while preparing statement: " << sqlite3_errmsg(db) << std::endl;
                continue;
            }

            sqlite3_bind_text(stmt, 1, uuidText.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, filePath.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(stmt, 3, fileSize);
            sqlite3_bind_text(stmt, 4, name.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 5, hashUtf8.constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 6, currentVaultPath.toUtf8().constData(), -1, SQLITE_TRANSIENT);

            if (sqlite3_step(stmt) != SQLITE_DONE)
                std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;

            sqlite3_finalize(stmt);

            // --- FINISH ---
            std::cout << "Datei erfolgreich hinzugefügt: " << name.toStdString() << std::endl;
            QString uuidReaded = readUuidFromFile(filePath);
            qDebug() << "Ausgelesene UUID:" << uuidReaded;
        }
    }

    close();
}

void addFiles::onCancelClicked() {
    close();
}

QString addFiles::readUuidFromFile(const QString &filePath) {
    QString adsPath = filePath + ":IdentFS_UUID";  // ADS-Pfad
    QFile file(adsPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Fehler beim Öffnen des ADS für" << filePath;
        return QString();
    }

    QTextStream in(&file);
    QString line = in.readLine();
    file.close();

    // Annahme: Zeile hat Format "UUID=<uuid>"
    if (line.startsWith("UUID=")) {
        return line.mid(5); // alles nach "UUID="
    }

    return QString();
}

std::string addFiles::hashText(const std::string &inputText) {
    QByteArray hash = QCryptographicHash::hash(inputText, QCryptographicHash::Sha256);
    return std::string(hash.toHex());
}
