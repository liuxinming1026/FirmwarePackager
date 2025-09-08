#include "ProjectSettingsDialog.h"

#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>
#include <QHBoxLayout>

ProjectSettingsDialog::ProjectSettingsDialog(const QString& root, const QString& output, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Project Settings");

    auto *layout = new QFormLayout(this);

    rootEdit = new QLineEdit(root);
    auto *rootBtn = new QPushButton("Browse");
    connect(rootBtn, &QPushButton::clicked, this, &ProjectSettingsDialog::browseRoot);
    auto *rootLayout = new QHBoxLayout;
    rootLayout->addWidget(rootEdit);
    rootLayout->addWidget(rootBtn);
    layout->addRow("Root", rootLayout);

    outputEdit = new QLineEdit(output);
    auto *outBtn = new QPushButton("Browse");
    connect(outBtn, &QPushButton::clicked, this, &ProjectSettingsDialog::browseOutput);
    auto *outLayout = new QHBoxLayout;
    outLayout->addWidget(outputEdit);
    outLayout->addWidget(outBtn);
    layout->addRow("Output", outLayout);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QString ProjectSettingsDialog::rootDir() const { return rootEdit->text(); }
QString ProjectSettingsDialog::outputDir() const { return outputEdit->text(); }

void ProjectSettingsDialog::browseRoot() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Root Directory");
    if (!dir.isEmpty())
        rootEdit->setText(dir);
}

void ProjectSettingsDialog::browseOutput() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory");
    if (!dir.isEmpty())
        outputEdit->setText(dir);
}

