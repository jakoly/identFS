// Local/Project headers
#include "addfiles.h"
#include "introdialog.h"
#include "mainwindow.h"
#include "settings.h"
#include "./ui_mainwindow.h"

// Qt headers
#include <QDebug>
#include <QFont>
#include <QIcon>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QThread>
#include <QtSql/QSql>

// C++ Standard Library headers
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <ctime>


std::atomic<bool> running{true};  // Für Stop
std::atomic<bool> paused{false};  // Für Pause

std::mutex mtx;
std::condition_variable cv;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ---- Connect slots ----
    connect(ui->pushButtonCreateProject, &QPushButton::clicked, this, &MainWindow::newProject);
    connect(ui->pushButtonAddFiles, &QPushButton::clicked, this, &MainWindow::openAddFilesWindow);
    connect(ui->projectList, &QListWidget::itemClicked, this, &MainWindow::onProjectItemClicked);
    connect(ui->listWidgetFiles, &QListWidget::itemClicked, this, &MainWindow::onFileClicked);
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &MainWindow::onCancelClicked);
    connect(ui->pushButtonOk, &QPushButton::clicked, this, &MainWindow::onOkClicked);
    connect(ui->pushButtonRecycle, &QPushButton::clicked, this, &MainWindow::onRecycleClicked);
    connect(ui->pushButtonSettings, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    connect(ui->pushButtonRepareFiles, &QPushButton::clicked, this, &MainWindow::repaireFiles);

    // ---- open SQLite DB ----
    db = nullptr;
    QString dbFolder = QCoreApplication::applicationDirPath() + "/database";
    QString dbPath   = dbFolder + "/identfs.db";

    QDir dir(QCoreApplication::applicationDirPath());
    if (!dir.exists("database")) dir.mkdir("database");

    int rc = sqlite3_open(dbPath.toUtf8().constData(), &db);
    if(rc) {
        std::cerr << "Database can't be opened: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Database opened successfully!" << std::endl;
    }

    // --- prepare UI ---
    turnAddProjectInvisible();

    // --- read Install-Pfad ---
    std::ifstream in(QCoreApplication::applicationDirPath().toStdString() + "/settings/stdPath.txt");
    std::string tempString;
    std::getline(in, tempString);
    installPath = QString::fromStdString(tempString).trimmed();

    QSettings settings;
    if (settings.value("intro/shown", false).toBool()) {
        //Introduction anzeigen
        qDebug() << "Show Introd.";

        introdialog dlg(this);
        dlg.exec();

        QSettings settings;
        settings.setValue("intro/shown", true);
    }


    // --- start Background-Thread ---
    startWorkerThread();
}

MainWindow::~MainWindow()
{
    stopWorker();
    if(db) sqlite3_close(db);
    delete ui;
}

std::string MainWindow::guidToString(const GUID& guid) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0')
       << std::setw(8) << guid.Data1 << "-"
       << std::setw(4) << guid.Data2 << "-"
       << std::setw(4) << guid.Data3 << "-";
    for(int i=0; i<2; i++) ss << std::setw(2) << static_cast<int>(guid.Data4[i]);
    ss << "-";
    for(int i=2; i<8; i++) ss << std::setw(2) << static_cast<int>(guid.Data4[i]);
    return ss.str();
}

void MainWindow::newProject()
{
    QString projectNameInput = ui->lineEditProjectName->text();
    std::string projectName = projectNameInput.toStdString();
    if(projectName.empty()) return;

    // generate UUID
    GUID guid;
    if (CoCreateGuid(&guid) != S_OK) {
        std::cerr << "Error while creating the uuid!" << std::endl;
        return;
    }
    std::string UUID = guidToString(guid);
    std::cout << "Project-UUID: " << UUID << std::endl;
    std::cout << "Name: " << projectName << std::endl;

    // --- Vault-Ordner erstellen ---
    std::filesystem::path path;
    try {
        // Pfad zusammensetzen (mit ":" für Windows-Laufwerke)
        path = std::filesystem::path((installPath).toStdString())
                                     / "vault" / UUID;

        // Ordner erstellen
        if (std::filesystem::create_directories(path)) {
            std::cout << "Ordner erstellt: " << path << std::endl;
        } else {
            std::cout << "Ordner existiert bereits oder konnte nicht erstellt werden." << std::endl;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Fehler: " << e.what() << std::endl;
    }

    //Generate Timestamp
    time_t timestamp;
    time(&timestamp);


    // Insert in DB
    sqlite3_stmt* stmt;
    const char* sqlInsert = "INSERT INTO projects (project_uuid, name, vault_path, create_date, modification_date) VALUES (?, ?, ?, ?, ?);";
    int rc = sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, nullptr);
    if(rc != SQLITE_OK) {
        std::cerr << "Error while preparing: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, UUID.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, projectName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, path.string().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, ctime(&timestamp), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, ctime(&timestamp), -1, SQLITE_TRANSIENT);

    if(sqlite3_step(stmt) != SQLITE_DONE)
        std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;

    sqlite3_finalize(stmt);




    std::cout << "Projekt erfolgreich hinzugefuegt!" << std::endl;
    updateProjectList();

    // LineEdit leeren
    ui->lineEditProjectName->clear();
}

void MainWindow::openAddFilesWindow()
{
    // Modelless-Fenster (kann parallel zu MainWindow benutzt werden)
    addFiles *window = new addFiles(this);  // "this" als Parent optional
    window->show();  // zeigt das Fenster
}

void MainWindow::updateFileList() {
    updateCounter = updateCounter + 1;

    ui->listWidgetFiles->clear();
    ui->listWidgetFiles->addItem(new QListWidgetItem(tr("Alle Dateien ⤵️")));
    ui->listWidgetFilesInProject->clear();
    ui->listWidgetFilesInProject->addItem(new QListWidgetItem(tr("Projektdateien ⤵️")));

    tempFileUUIDs.clear();
    qDebug() << tempFileUUIDs << " is empty now";
    qDebug() << "Update File List";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, "SELECT file_uuid, name, last_path, size, active FROM files;", -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qDebug() << "SQLite-Fehler";
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        QString uuid = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        QString name = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        QString lastPath = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        QString size = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        int active = sqlite3_column_int(stmt, 4);

        QListWidgetItem *item = new QListWidgetItem(QIcon(":/icons/icons/file.png"), name);

        std::fstream file(lastPath.toStdString(), std::ios::in);
        if (file) {  // Check if stream is successfull
            qDebug() << "Datei existiert." << updateCounter;
            item->setFlags(item->flags() | Qt::ItemIsEnabled);

            sqlite3_stmt* stmt;
            const char* sqlDelete =
                "DELETE FROM repaire_files WHERE file_uuid = ?;";

            int rc = sqlite3_prepare_v2(db, sqlDelete, -1, &stmt, nullptr);
            if (rc != SQLITE_OK) {
                std::cerr << "Prepare failed: "
                          << sqlite3_errmsg(db) << std::endl;
                return;
            }

            QByteArray uuidUtf8 = uuid.toUtf8();
            sqlite3_bind_text(
                stmt,
                1,
                uuidUtf8.constData(),
                uuidUtf8.size(),
                SQLITE_TRANSIENT
                );

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                std::cerr << "Delete failed: "
                          << sqlite3_errmsg(db) << std::endl;
            } else {
                if (sqlite3_changes(db) == 0) {
                    // UUID war NICHT in der Tabelle
                    std::cout << "UUID not found, nothing deleted\n";
                } else {
                    // UUID war vorhanden
                    std::cout << "UUID deleted successfully\n";
                }
            }

            sqlite3_finalize(stmt);

        } else {
            qDebug() << "Datei existiert nicht.\n" << updateCounter;
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            item->setIcon(QIcon(":/icons/icons/question.png"));

            // --- Zur Database hizufügen ---


            sqlite3_stmt* stmt;
            const char* sqlInsert =
                "INSERT OR IGNORE INTO repaire_files (file_uuid) VALUES (?);";

            int rc = sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, nullptr);
            if(rc != SQLITE_OK) {
                std::cerr << "Error while preparing: " << sqlite3_errmsg(db) << std::endl;
                return;
            }

            sqlite3_bind_text(stmt, 1, uuid.toUtf8().constData(), -1, SQLITE_TRANSIENT);

            if(sqlite3_step(stmt) != SQLITE_DONE)
                std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;

            sqlite3_finalize(stmt);

        }

        QVariantMap data;
        data["name"] = name;
        data["uuid"] = uuid;
        data["lastPath"] = lastPath;
        data["size"] = size;
        data["active"] = active;

        item->setData(Qt::UserRole, data);
        ui->listWidgetFiles->addItem(item);
    }

    sqlite3_finalize(stmt);
}

void MainWindow::updateProjectList() {

    // QListWidget Item
    ui->projectList->setIconSize(QSize(32, 32));
    QFont font = ui->projectList->font();
    font.setPointSize(14);
    ui->projectList->setFont(font);
    ui->projectList->clear();


    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, "SELECT project_uuid, name, vault_path, create_date, modification_date FROM projects;", -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qDebug() << "sqLite-Fehler";
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* uuidText = sqlite3_column_text(stmt, 0);
        const unsigned char* nameText = sqlite3_column_text(stmt, 1);
        const unsigned char* vaultText = sqlite3_column_text(stmt, 2);
        const unsigned char* create_dateText = sqlite3_column_text(stmt, 3);
        const unsigned char* modification_dateText = sqlite3_column_text(stmt, 4);

        QString uuid = reinterpret_cast<const char*>(uuidText);
        QString name = reinterpret_cast<const char*>(nameText);
        QString vaultPath = reinterpret_cast<const char*>(vaultText);
        QString create_date = reinterpret_cast<const char*>(create_dateText);
        QString modification_date = reinterpret_cast<const char*>(modification_dateText);
        qDebug() << vaultPath;

        QListWidgetItem *item = new QListWidgetItem(QIcon(":/icons/icons/project.png"), name);
        QVariantMap data;
        data["name"] = name;
        data["uuid"] = uuid;
        data["vault_path"] = vaultPath;
        data["createDate"] = create_date;
        data["modification_date"] = modification_date;

        item->setData(Qt::UserRole, data);

        ui->projectList->addItem(item);
    }

    sqlite3_finalize(stmt);
}

void MainWindow::turnAddProjectInvisible() {
    ui->groupNewProject->setVisible(false);
    projectUUID = "";
    projectName = "";
    projectCreated = "";
    projectModificated = "";
    projectVaultPath = "";
    tempFileUUIDs.clear();

    ui->groupNewProject->setStyleSheet("QGroupBox { border: none; }");
    ui->labelProjectName->setText(tr("Willkommen bei identFS!"));

    updateFileList();
    updateProjectList();
    qDebug() << tempFileUUIDs << " is empty now";

    qDebug() << "Kein Projekt ausgewählt.";
}

void MainWindow::onProjectItemClicked(QListWidgetItem *item)
{
    updateFileList();
    pauseWorker();

    QVariantMap data = item->data(Qt::UserRole).toMap();
    QString name = data["name"].toString();
    QString uuid = data["uuid"].toString();
    QString vaultPath = data["vaultPath"].toString();
    QString createDate = data["createDate"].toString();
    QString modification_date = data["modification_date"].toString();

    projectUUID = uuid;
    projectName = name;
    projectCreated = createDate;
    projectModificated = modification_date;
    projectVaultPath = vaultPath;

    loadProjectFiles(projectUUID);

    qDebug() << "Item geklickt: " << projectName << " Vault-Path:" << projectVaultPath;

    ui->groupNewProject->setVisible(true);

    ui->labelProjectName->setText(QString("%1").arg(projectName));
    ui->labelCreated->setText(QString(tr("Projekt erstellt:    %1")).arg(projectCreated));
    ui->labelLastChange->setText(QString(tr("Projekt geändert:     %1")).arg(projectModificated));
}

void MainWindow::onFileClicked(QListWidgetItem *item) {
    QListWidgetItem *newItem = item->clone();
    ui->listWidgetFilesInProject->addItem(newItem);

    QVariantMap data = item->data(Qt::UserRole).toMap();
    QString uuid = data["uuid"].toString();
    tempFileUUIDs.append(uuid);
    qDebug() << tempFileUUIDs;
}

void MainWindow::onOkClicked() {
    // --- Datenbankeintrag erstellen ---
    const char* sqlInsert =
        "INSERT INTO file_to_project (file_uuid, project_uuid) VALUES (?, ?);";

    sqlite3_stmt* stmtInsert = nullptr;
    if (sqlite3_prepare_v2(db, sqlInsert, -1, &stmtInsert, nullptr) != SQLITE_OK) {
        qDebug() << "Prepare failed:" << sqlite3_errmsg(db);
        return;
    }

    for (const QString &fileUUID : tempFileUUIDs) {
        QByteArray fileUuidUtf8 = fileUUID.toUtf8();
        QByteArray projectUuidUtf8 = projectUUID.toUtf8();

        sqlite3_bind_text(stmtInsert, 1, fileUuidUtf8.constData(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmtInsert, 2, projectUuidUtf8.constData(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmtInsert) != SQLITE_DONE)
            qDebug() << "Insert failed:" << sqlite3_errmsg(db);

        sqlite3_reset(stmtInsert);
        sqlite3_clear_bindings(stmtInsert);

        qDebug() << fileUUID << "was added to" << projectUUID;

        // --- Vault-Kopie ---
        QString lastPath;

        sqlite3_stmt* stmtSelect = nullptr;
        if (sqlite3_prepare_v2(db,
                               "SELECT last_path FROM files WHERE file_uuid = ?",
                               -1,
                               &stmtSelect, nullptr) == SQLITE_OK) {

            sqlite3_bind_text(stmtSelect, 1, fileUuidUtf8.constData(), -1, SQLITE_TRANSIENT);

            if (sqlite3_step(stmtSelect) == SQLITE_ROW) {
                const unsigned char* text = sqlite3_column_text(stmtSelect, 0);
                if (text)
                    lastPath = QString::fromUtf8(reinterpret_cast<const char*>(text));
            }

            sqlite3_finalize(stmtSelect);
        }

        if (lastPath.isEmpty()) {
            qDebug() << "Vault skipped: lastPath empty for" << fileUUID;
            continue;
        }

        if (!QFile::exists(lastPath)) {
            qDebug() << "Vault skipped: file does not exist:" << lastPath;
            continue;
        }

        QString tempPath = installPath + "/vault/" + projectUUID;
        if (!QDir().mkpath(tempPath)) {
            qDebug() << "Failed to create vault folder:" << tempPath;
            continue;
        }

        QFileInfo fi(lastPath);
        QString sourceDir = fi.absolutePath();
        QString fileName  = fi.fileName();

        QStringList args = {
            sourceDir,
            tempPath,
            fileName,
            "/COPY:DAT", // nur Daten, Attribute, Zeitstempel
            "/R:0",
            "/W:0"
        };

        QProcess proc;
        proc.start("robocopy", args);
        proc.waitForFinished(-1);

        int exitCode = proc.exitCode();
        if (exitCode >= 8) {
            qDebug() << "Robocopy failed:" << exitCode << "for file:" << fileUUID;
            continue;
        }

        qDebug() << fileUUID << "was vaulted!";
    }

    sqlite3_finalize(stmtInsert);

    QMessageBox::information(
        this,
        tr("Dateien wurden hinzugefügt"),
        tr("Die Dateien wurden erfolgreich zum Projekt hinzugefügt.\nSie sind im Vault gesichert.")
        );

    turnAddProjectInvisible();
    resumeWorker();
}

void MainWindow::onCancelClicked() {
    turnAddProjectInvisible();
    resumeWorker();
}

void MainWindow::loadProjectFiles(const QString &projectUuid) {
    ui->listWidgetFilesInProject->clear(); // vorher leeren

    // 1 Alle file_uuid für das Projekt aus file_to_project holen
    QStringList fileUUIDs;

    const char* sqlFilesToProject = "SELECT file_uuid FROM file_to_project WHERE project_uuid = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sqlFilesToProject, -1, &stmt, nullptr) != SQLITE_OK) {
        qDebug() << "Prepare failed:" << sqlite3_errmsg(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, projectUuid.toUtf8().constData(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        QString fileUuid = QString::fromUtf8(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))
            );
        fileUUIDs.append(fileUuid);
    }

    sqlite3_finalize(stmt);

    // 2️ Für jede file_uuid den Namen aus files holen und Item hinzufügen
    const char* sqlFileName = "SELECT name FROM files WHERE file_uuid = ?;";

    for (const QString &fileUuid : fileUUIDs) {
        sqlite3_stmt* stmtName = nullptr;
        if (sqlite3_prepare_v2(db, sqlFileName, -1, &stmtName, nullptr) != SQLITE_OK) {
            qDebug() << "Prepare failed:" << sqlite3_errmsg(db);
            continue;
        }

        sqlite3_bind_text(stmtName, 1, fileUuid.toUtf8().constData(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmtName) == SQLITE_ROW) {
            QString fileName = QString::fromUtf8(
                reinterpret_cast<const char*>(sqlite3_column_text(stmtName, 0))
                );

            // Item erstellen und zum ListWidget hinzufügen
            QListWidgetItem *item = new QListWidgetItem(fileName);
            ui->listWidgetFilesInProject->addItem(item);
        }

        sqlite3_finalize(stmtName);
    }
}

void MainWindow::onRecycleClicked() {
    // --- Sicherheitsabfrage 1 ---
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
        this,
        tr("Sicherheitsabfrage"),
        tr("Willst du dieses Projekt wirklich löschen? \nDas Projekt kann nicht wiederhergestellt werden!"),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes) {
        turnAddProjectInvisible();
        return;
    }

    // --- Sicherheitsabfrage 2 ---
    reply = QMessageBox::question(
        this,
        tr("Sicherheitsabfrage"),
        tr("Wirklich sicher?????"),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes) {
        turnAddProjectInvisible();
        return;
    }

    // --- Datenbankeinträge löschen ---
    auto deleteProjectEntry = [&](const QString &sqlQuery){
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, sqlQuery.toUtf8().constData(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            qDebug() << "Prepare failed:" << sqlite3_errmsg(db);
            return;
        }
        QByteArray uuidUtf8 = projectUUID.toUtf8();
        sqlite3_bind_text(stmt, 1, uuidUtf8.constData(), -1, SQLITE_TRANSIENT);
        if(sqlite3_step(stmt) != SQLITE_DONE)
            qDebug() << "Delete failed:" << sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
    };

    deleteProjectEntry("DELETE FROM projects WHERE project_uuid = ?");
    deleteProjectEntry("DELETE FROM file_to_project WHERE project_uuid = ?");

    qDebug() << "Projekt" << projectUUID << "wurde aus der Datenbank entfernt";

    // --- Vault-Ordner deaktivieren ---
    const char* sqlUpdateProject = "UPDATE projects SET active = ? WHERE project_uuid = ?;";
    sqlite3_stmt* stmt = nullptr;

    // Statement vorbereiten
    if (sqlite3_prepare_v2(db, sqlUpdateProject, -1, &stmt, nullptr) != SQLITE_OK) {
        qDebug() << "Prepare failed:" << sqlite3_errmsg(db);
        return;
    }

    // --- GUI aufräumen ---
    turnAddProjectInvisible();
    updateFileList();
    updateProjectList();
}

void MainWindow::onSettingsClicked() {
    // Modelless-Fenster (kann parallel zu MainWindow benutzt werden)
    settings *window = new settings(this);  // "this" als Parent optional
    window->show();  // zeigt das Fenster
}

void MainWindow::repeatTask()
{
    while (running) {

        // -------- Pause / Stop --------
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] {
                return !paused || !running;
            });
        }
        if (!running)
            break;

        // -------- Hintergrund-Logik (KEIN GUI!) --------
        bool tableIsEmpty = true;

        sqlite3_stmt* stmt = nullptr;
        const char* sql =
            "SELECT EXISTS (SELECT 1 FROM repaire_files LIMIT 1);";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                tableIsEmpty = (sqlite3_column_int(stmt, 0) == 0);
            }
            sqlite3_finalize(stmt);
        }

        // -------- GUI-Update IM GUI-THREAD --------
        QMetaObject::invokeMethod(
            this,
            [this, tableIsEmpty]() {
                qDebug() << "File/Project update";

                updateFileList();
                updateProjectList();
                loadProjectFiles(projectUUID);

                ui->pushButtonRepareFiles->setEnabled(!tableIsEmpty);
            },
            Qt::QueuedConnection
            );

        // -------- 5 Sekunden warten (reaktiv) --------
        for (int i = 0; i < 50; ++i) {   // 50 × 100 ms
            if (!running)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void MainWindow::repaireFiles() {
    sqlite3_stmt* stmtOuter;
    int rc = sqlite3_prepare_v2(db, "SELECT file_uuid FROM repaire_files;", -1, &stmtOuter, nullptr);
    if (rc != SQLITE_OK) {
        qDebug() << "SQLite-Fehler bei repaire_files";
        return;
    }

    while ((rc = sqlite3_step(stmtOuter)) == SQLITE_ROW) {
        QString file_uuid = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmtOuter, 0)));

        // Inneres Statement: current_vault_path und last_path abfragen
        sqlite3_stmt* stmtInner;
        rc = sqlite3_prepare_v2(db,
                                "SELECT current_vault_path, last_path FROM files WHERE file_uuid=?",
                                -1, &stmtInner, nullptr);
        if (rc != SQLITE_OK) {
            qDebug() << "SQLite-Fehler bei files";
            continue;
        }

        // Bind den file_uuid Wert
        sqlite3_bind_text(stmtInner, 1, file_uuid.toUtf8().constData(), -1, SQLITE_TRANSIENT);

        if ((rc = sqlite3_step(stmtInner)) == SQLITE_ROW) {
            QString currentPath = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmtInner, 0)));
            QString lastPath    = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmtInner, 1)));

            qDebug() << "UUID:" << file_uuid << "currentPath:" << currentPath << "lastPath:" << lastPath;

            copyWithRobocopy(currentPath, lastPath);

            qDebug() << "Die Datei " << file_uuid << " wurde erfolgreich nach " << lastPath << "kopiert";
        }

        sqlite3_finalize(stmtInner); // wichtig
    }

    sqlite3_finalize(stmtOuter);
}

void MainWindow::copyWithRobocopy(const QString& currentPath, const QString& lastPath) {
    QFileInfo srcInfo(currentPath);
    QFileInfo dstInfo(lastPath);

    if (!srcInfo.exists()) {
        qDebug() << "Source file does not exist:" << currentPath;
        return;
    }

    QDir().mkpath(dstInfo.absolutePath()); // Zielordner erstellen

    QProcess proc;
    QStringList args;
    args << srcInfo.absolutePath().replace("/", "\\")       // Quellordner
         << dstInfo.absolutePath().replace("/", "\\")       // Zielordner
         << srcInfo.fileName()                               // Datei
         << "/COPY:DAT"                                     // Daten, Attribute, Zeitstempel
         << "/R:0" << "/W:0";

    proc.start("robocopy", args);
    proc.waitForFinished(-1);

    QString output = proc.readAllStandardOutput();
    qDebug() << output;
}

void MainWindow::startWorkerThread() {
    std::thread([this]() {
        while (running) {
            // --- Pause ---
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [](){ return !paused || !running; });
            }
            if (!running) break;

            // --- DB-Check ---
            bool tableIsEmpty = true;
            sqlite3_stmt* stmt;
            const char* sql = "SELECT EXISTS (SELECT 1 FROM repaire_files LIMIT 1);";
            if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                if(sqlite3_step(stmt) == SQLITE_ROW)
                    tableIsEmpty = (sqlite3_column_int(stmt, 0) == 0);
                sqlite3_finalize(stmt);
            }

            // --- GUI-Updates ---
            QMetaObject::invokeMethod(this, [this, tableIsEmpty](){
                ui->pushButtonRepareFiles->setEnabled(!tableIsEmpty);
                updateFileList();
                updateProjectList();
            }, Qt::QueuedConnection);

            // --- Intervall 5 Sekunden mit schneller Reaktion auf Pause/Stop ---
            for(int i = 0; i < 50; ++i) {
                if(!running) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }).detach();
}

void MainWindow::pauseWorker() {
    qDebug() << "Worker paused";
    paused = true;
}

void MainWindow::resumeWorker() {
    qDebug() << "Worker resumed";
    paused = false;
    cv.notify_all();
}

void MainWindow::stopWorker() {
    qDebug() << "Worker stoped";
    running = false;
    cv.notify_all();
}

void MainWindow::createSetting() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS file_to_project ("
        "file_uuid TEXT NOT NULL,"
        "project_uuid TEXT NOT NULL"
        ");"

        "CREATE TABLE IF NOT EXISTS files ("
        "file_uuid TEXT NOT NULL UNIQUE,"
        "name TEXT NOT NULL,"
        "last_path TEXT,"
        "size REAL,"
        "active INTEGER NOT NULL,"
        "file_hash BLOB NOT NULL,"
        "current_vault_path TEXT NOT NULL"
        ");"

        "CREATE TABLE IF NOT EXISTS projects ("
        "project_uuid TEXT NOT NULL UNIQUE,"
        "name TEXT NOT NULL,"
        "vault_path TEXT NOT NULL,"
        "create_date TEXT NOT NULL,"
        "modification_date TEXT NOT NULL"
        ");"

        "CREATE TABLE IF NOT EXISTS repaire_files ("
        "file_uuid TEXT NOT NULL"
        ");";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL Fehler: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "Tabellen erfolgreich erstellt\n";
    }
}

