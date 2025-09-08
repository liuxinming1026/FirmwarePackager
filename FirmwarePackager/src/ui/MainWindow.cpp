#include "MainWindow.h"
#include "MappingDialog.h"
#include "ProjectSettingsDialog.h"

#include <QToolBar>
#include <QDockWidget>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QAction>
#include <QPushButton>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), guiLogger(nullptr) {
    setWindowTitle("Upgrade Builder");

    auto *splitter = new QSplitter(Qt::Vertical, this);

    model = new QStandardItemModel(0, 3, this);
    model->setHeaderData(0, Qt::Horizontal, "Path");
    model->setHeaderData(1, Qt::Horizontal, "ID");
    model->setHeaderData(2, Qt::Horizontal, "Hash");

    tableView = new QTableView;
    tableView->setModel(model);
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

    QAction *buildAct = tb->addAction("Build");
    connect(buildAct, &QAction::triggered, this, &MainWindow::buildPackage);

    QAction *settingsAct = tb->addAction("Settings");
    connect(settingsAct, &QAction::triggered, this, &MainWindow::openSettings);

    QAction *editAct = tb->addAction("Edit Mapping");
    connect(editAct, &QAction::triggered, this, &MainWindow::editMapping);

    guiLogger = new GuiLogger(logPane);
    packager = std::make_unique<core::Packager>(scanner, hasher, manifest, script, idGen, *guiLogger);
}

void MainWindow::populateTable(const core::Project& project) {
    model->setRowCount(0);
    int row = 0;
    for (const auto& file : project.files) {
        model->insertRow(row);
        model->setData(model->index(row, 0), QString::fromStdString(file.path.string()));
        model->setData(model->index(row, 1), QString::fromStdString(file.id));
        model->setData(model->index(row, 2), QString::fromStdString(file.hash));
        ++row;
    }
}

void MainWindow::openRoot() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Project Root");
    if (dir.isEmpty())
        return;
    rootEdit->setText(dir);
    currentProject = packager->buildProject(dir.toStdString(), core::Scanner::PathList{});
    populateTable(currentProject);
}

void MainWindow::buildPackage() {
    if (rootEdit->text().isEmpty() || outputEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Missing Paths", "Please specify both root and output directories.");
        return;
    }
    packager->package(rootEdit->text().toStdString(), outputEdit->text().toStdString(), core::Scanner::PathList{});
}

void MainWindow::openSettings() {
    ProjectSettingsDialog dlg(rootEdit->text(), outputEdit->text(), this);
    if (dlg.exec() == QDialog::Accepted) {
        rootEdit->setText(dlg.rootDir());
        outputEdit->setText(dlg.outputDir());
        if (!dlg.rootDir().isEmpty()) {
            currentProject = packager->buildProject(dlg.rootDir().toStdString(), core::Scanner::PathList{});
            populateTable(currentProject);
        }
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
        model->setData(model->index(row, 1), QString::fromStdString(entry.id));
    }
}

