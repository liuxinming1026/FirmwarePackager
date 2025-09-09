#include "MainWindow.h"
#include "MappingDialog.h"
#include "ProjectSettingsDialog.h"
#include "BatchEditDialog.h"

#include <QToolBar>
#include <QDockWidget>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QAction>
#include <QPushButton>
#include <QStringList>
#include <QMenuBar>
#include <QTemporaryDir>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QCoreApplication>
#include <fstream>
#include <vector>
#include <iterator>
#include <filesystem>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), guiLogger(nullptr) {
    setWindowTitle("Upgrade Builder");

    // file menu
    QMenu *fileMenu = menuBar()->addMenu("File");
    QAction *newAct = fileMenu->addAction("New");
    connect(newAct, &QAction::triggered, this, &MainWindow::newProject);
    QAction *openProjAct = fileMenu->addAction("Open");
    connect(openProjAct, &QAction::triggered, this, &MainWindow::openProject);
    QAction *saveProjAct = fileMenu->addAction("Save");
    connect(saveProjAct, &QAction::triggered, this, &MainWindow::saveProject);

    auto *splitter = new QSplitter(Qt::Vertical, this);

    model = new QStandardItemModel(0, 9, this);
    model->setHeaderData(0, Qt::Horizontal, "Path");
    model->setHeaderData(1, Qt::Horizontal, "Target");
    model->setHeaderData(2, Qt::Horizontal, "ID");
    model->setHeaderData(3, Qt::Horizontal, "Mode");
    model->setHeaderData(4, Qt::Horizontal, "Owner");
    model->setHeaderData(5, Qt::Horizontal, "Group");
    model->setHeaderData(6, Qt::Horizontal, "Recursive");
    model->setHeaderData(7, Qt::Horizontal, "Excludes");
    model->setHeaderData(8, Qt::Horizontal, "MD5");

    tableView = new QTableView;
    tableView->setModel(model);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(tableView, &QTableView::doubleClicked, this, &MainWindow::editMapping);
    splitter->addWidget(tableView);

    logPane = new QPlainTextEdit;
    logPane->setReadOnly(true);
    splitter->addWidget(logPane);

    setCentralWidget(splitter);

    // settings dock
    auto *dock = new QDockWidget("Settings", this);
    QWidget *settingsWidget = new QWidget;
    auto *form = new QFormLayout(settingsWidget);

    rootEdit = new QLineEdit;
    auto *rootBtn = new QPushButton("Browse");
    connect(rootBtn, &QPushButton::clicked, this, &MainWindow::openRoot);
    auto *rootLayout = new QHBoxLayout;
    rootLayout->addWidget(rootEdit);
    rootLayout->addWidget(rootBtn);
    form->addRow("Root", rootLayout);

    outputEdit = new QLineEdit;
    auto *outBtn = new QPushButton("Browse");
    connect(outBtn, &QPushButton::clicked, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory");
        if (!dir.isEmpty())
            outputEdit->setText(dir);
    });
    auto *outLayout = new QHBoxLayout;
    outLayout->addWidget(outputEdit);
    outLayout->addWidget(outBtn);
    form->addRow("Output", outLayout);

    dock->setWidget(settingsWidget);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    // toolbar actions
    auto *tb = addToolBar("Main");
    QAction *openAct = tb->addAction("Open");
    connect(openAct, &QAction::triggered, this, &MainWindow::openRoot);

    QAction *addFileAct = tb->addAction("Add File");
    connect(addFileAct, &QAction::triggered, this, &MainWindow::addFile);

    QAction *addFolderAct = tb->addAction("Add Folder");
    connect(addFolderAct, &QAction::triggered, this, &MainWindow::addFolder);

    QAction *batchAct = tb->addAction("Batch Edit");
    connect(batchAct, &QAction::triggered, this, &MainWindow::batchEdit);

    QAction *previewAct = tb->addAction("Preview Script");
    connect(previewAct, &QAction::triggered, this, &MainWindow::previewScript);

    QAction *buildAct = tb->addAction("Build");
    connect(buildAct, &QAction::triggered, this, &MainWindow::buildPackage);

    QAction *settingsAct = tb->addAction("Settings");
    connect(settingsAct, &QAction::triggered, this, &MainWindow::openSettings);

    QAction *editAct = tb->addAction("Edit Mapping");
    connect(editAct, &QAction::triggered, this, &MainWindow::editMapping);

    guiLogger = new GuiLogger(logPane);
    auto tplRoot = std::filesystem::path(QCoreApplication::applicationDirPath().toStdString()) / "templates";
    packager = std::make_unique<core::Packager>(scanner, hasher, manifest, script, idGen, *guiLogger, tplRoot);
}

void MainWindow::newProject() {
    currentProject = core::Project();
    rootEdit->clear();
    outputEdit->clear();
    model->setRowCount(0);
}

void MainWindow::openProject() {
    QString file = QFileDialog::getOpenFileName(this, "Open Project", QString(), "Project Files (*.json)");
    if (file.isEmpty())
        return;
    try {
        currentProject = serializer.load(file.toStdString());
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
        return;
    }
    rootEdit->setText(QString::fromStdString(currentProject.rootDir.string()));
    outputEdit->setText(QString::fromStdString(currentProject.outputDir.string()));
    populateTable(currentProject);
}

void MainWindow::saveProject() {
    QString file = QFileDialog::getSaveFileName(this, "Save Project", QString(), "Project Files (*.json)");
    if (file.isEmpty())
        return;
    currentProject.rootDir = rootEdit->text().toStdString();
    currentProject.outputDir = outputEdit->text().toStdString();
    try {
        serializer.save(currentProject, file.toStdString());
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void MainWindow::populateTable(const core::Project& project) {
    model->setRowCount(0);
    int row = 0;
    for (const auto& file : project.files) {
        model->insertRow(row);
        model->setData(model->index(row, 0), QString::fromStdString(file.path.string()));
        model->setData(model->index(row, 1), QString::fromStdString(file.dest.string()));
        model->setData(model->index(row, 2), QString::fromStdString(file.id));
        model->setData(model->index(row, 3), QString::fromStdString(file.mode));
        model->setData(model->index(row, 4), QString::fromStdString(file.owner));
        model->setData(model->index(row, 5), QString::fromStdString(file.group));
        bool isDir = file.recursive;
        if (!isDir) {
            std::error_code ec; // avoid exceptions
            isDir = std::filesystem::is_directory(project.rootDir / file.path, ec);
        }
        model->setData(model->index(row, 6), isDir ? "Yes" : "No");
        if (isDir) {
            model->setData(model->index(row, 7), "");
        } else {
            QStringList exList;
            for (const auto& ex : file.excludes)
                exList << QString::fromStdString(ex.string());
            model->setData(model->index(row, 7), exList.join(","));
        }
        model->setData(model->index(row, 8), QString::fromStdString(file.hash));
        ++row;
    }
}

void MainWindow::openRoot() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Project Root");
    if (dir.isEmpty())
        return;
    rootEdit->setText(dir);
    currentProject = packager->buildProject(dir.toStdString(), core::Scanner::PathList{});
    currentProject.outputDir = outputEdit->text().toStdString();
    populateTable(currentProject);
}

void MainWindow::buildPackage() {
    if (rootEdit->text().isEmpty() || outputEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Missing Paths", "Please specify both root and output directories.");
        return;
    }
    currentProject.rootDir = rootEdit->text().toStdString();
    currentProject.outputDir = outputEdit->text().toStdString();
    packager->package(currentProject);
}

void MainWindow::openSettings() {
    ProjectSettingsDialog dlg(rootEdit->text(), outputEdit->text(),
                              QString::fromStdString(currentProject.pkgId), this);
    if (dlg.exec() == QDialog::Accepted) {
        rootEdit->setText(dlg.rootDir());
        outputEdit->setText(dlg.outputDir());
        if (!dlg.rootDir().isEmpty()) {
            currentProject = packager->buildProject(dlg.rootDir().toStdString(), core::Scanner::PathList{});
            currentProject.outputDir = dlg.outputDir().toStdString();
        } else {
            currentProject.outputDir = dlg.outputDir().toStdString();
        }
        currentProject.pkgId = dlg.pkgId().toStdString();
        populateTable(currentProject);
    }
}

void MainWindow::editMapping() {
    QModelIndex index = tableView->currentIndex();
    if (!index.isValid())
        return;
    int row = index.row();
    if (row < 0 || row >= static_cast<int>(currentProject.files.size()))
        return;
    core::FileEntry& entry = currentProject.files[row];
    MappingDialog dlg(entry, this);
    if (dlg.exec() == QDialog::Accepted) {
        model->setData(model->index(row, 1), QString::fromStdString(entry.dest.string()));
        model->setData(model->index(row, 2), QString::fromStdString(entry.id));
        model->setData(model->index(row, 3), QString::fromStdString(entry.mode));
        model->setData(model->index(row, 4), QString::fromStdString(entry.owner));
        model->setData(model->index(row, 5), QString::fromStdString(entry.group));
        bool isDir = entry.recursive;
        if (!isDir) {
            std::error_code ec;
            isDir = std::filesystem::is_directory(currentProject.rootDir / entry.path, ec);
        }
        model->setData(model->index(row, 6), isDir ? "Yes" : "No");
        if (isDir) {
            model->setData(model->index(row, 7), "");
        } else {
            QStringList exList;
            for (const auto& ex : entry.excludes)
                exList << QString::fromStdString(ex.string());
            model->setData(model->index(row, 7), exList.join(","));
        }
    }
}

void MainWindow::addFile() {
    if (rootEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Missing Root", "Please select project root first.");
        return;
    }
    QStringList files = QFileDialog::getOpenFileNames(this, "Select Files", rootEdit->text());
    if (files.isEmpty())
        return;
    std::filesystem::path root = rootEdit->text().toStdString();
    for (const auto& f : files) {
        std::filesystem::path abs = f.toStdString();
        std::error_code ec;
        auto rel = std::filesystem::relative(abs, root, ec);
        if (ec || rel.empty() || rel.native().rfind("..", 0) == 0)
            continue;
        std::ifstream in(abs, std::ios::binary);
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        std::string hash = hasher.md5(data);
        std::string id = idGen.generate();
        currentProject.files.emplace_back(rel, id, hash);
    }
    populateTable(currentProject);
}

void MainWindow::addFolder() {
    if (rootEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Missing Root", "Please select project root first.");
        return;
    }
    QString dir = QFileDialog::getExistingDirectory(this, "Select Folder", rootEdit->text());
    if (dir.isEmpty())
        return;
    std::filesystem::path root = rootEdit->text().toStdString();
    std::filesystem::path folder = dir.toStdString();
    std::error_code ec;
    auto rel = std::filesystem::relative(folder, root, ec);
    if (!ec && !rel.empty() && rel.native().rfind("..", 0) != 0) {
        core::FileEntry entry;
        entry.path = rel;
        entry.dest = rel;
        entry.recursive = true;
        entry.mode = "0755";
        currentProject.files.push_back(entry);
    }
    populateTable(currentProject);
}

void MainWindow::batchEdit() {
    auto rows = tableView->selectionModel()->selectedRows();
    if (rows.isEmpty())
        return;
    BatchEditDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    for (const auto& idx : rows) {
        int row = idx.row();
        if (row < 0 || row >= static_cast<int>(currentProject.files.size()))
            continue;
        auto& entry = currentProject.files[row];
        if (!dlg.prefix().isEmpty()) {
            std::filesystem::path pref = dlg.prefix().toStdString();
            entry.dest = pref / entry.dest;
            model->setData(model->index(row, 1), QString::fromStdString(entry.dest.string()));
        }
        if (!dlg.mode().isEmpty()) {
            entry.mode = dlg.mode().toStdString();
            model->setData(model->index(row, 3), dlg.mode());
        }
        if (!dlg.owner().isEmpty()) {
            entry.owner = dlg.owner().toStdString();
            model->setData(model->index(row, 4), dlg.owner());
        }
        if (!dlg.group().isEmpty()) {
            entry.group = dlg.group().toStdString();
            model->setData(model->index(row, 5), dlg.group());
        }
    }
}

void MainWindow::previewScript() {
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        QMessageBox::warning(this, "Error", "Unable to create temporary directory.");
        return;
    }
    try {
        script.write(currentProject, tempDir.path().toStdString(), idGen.generate());
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
        return;
    }
    QFile file(tempDir.path() + "/install.sh");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "install.sh not generated");
        return;
    }
    QString content = file.readAll();
    file.close();

    QDialog dlg(this);
    dlg.setWindowTitle("install.sh Preview");
    auto *layout = new QVBoxLayout(&dlg);
    auto *text = new QPlainTextEdit;
    text->setReadOnly(true);
    text->setPlainText(content);
    layout->addWidget(text);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);
    dlg.resize(600, 400);
    dlg.exec();
}

