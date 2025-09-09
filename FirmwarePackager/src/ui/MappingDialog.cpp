#include "MappingDialog.h"

#include <QFormLayout>
#include <QDialogButtonBox>
#include <QStringList>

MappingDialog::MappingDialog(core::FileEntry& entry, QWidget* parent)
    : QDialog(parent), fileEntry(entry) {
    setWindowTitle("Edit Mapping");

    auto *layout = new QFormLayout(this);

    auto *pathEdit = new QLineEdit(QString::fromStdString(fileEntry.path.string()));
    pathEdit->setReadOnly(true);
    layout->addRow("Path", pathEdit);

    idEdit = new QLineEdit(QString::fromStdString(fileEntry.id));
    layout->addRow("ID", idEdit);

    destEdit = new QLineEdit(QString::fromStdString(fileEntry.dest.string()));
    layout->addRow("Dest", destEdit);

    modeEdit = new QLineEdit(QString::fromStdString(fileEntry.mode));
    layout->addRow("Mode", modeEdit);

    ownerEdit = new QLineEdit(QString::fromStdString(fileEntry.owner));
    layout->addRow("Owner", ownerEdit);

    groupEdit = new QLineEdit(QString::fromStdString(fileEntry.group));
    layout->addRow("Group", groupEdit);

    recursiveCheck = new QCheckBox;
    recursiveCheck->setChecked(fileEntry.recursive);
    layout->addRow("Recursive", recursiveCheck);

    QStringList ex;
    for (const auto& e : fileEntry.excludes)
        ex << QString::fromStdString(e.string());
    excludesEdit = new QLineEdit(ex.join(","));
    excludesEdit->setEnabled(fileEntry.recursive);
    layout->addRow("Excludes", excludesEdit);
    connect(recursiveCheck, &QCheckBox::toggled, excludesEdit, &QWidget::setEnabled);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &MappingDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &MappingDialog::reject);
    layout->addWidget(buttons);
}

void MappingDialog::accept() {
    fileEntry.id = idEdit->text().toStdString();
    fileEntry.dest = destEdit->text().toStdString();
    fileEntry.mode = modeEdit->text().toStdString();
    fileEntry.owner = ownerEdit->text().toStdString();
    fileEntry.group = groupEdit->text().toStdString();
    fileEntry.recursive = recursiveCheck->isChecked();
    fileEntry.excludes.clear();
    const auto parts = excludesEdit->text().split(',', Qt::SkipEmptyParts);
    for (const auto& p : parts) {
        fileEntry.excludes.emplace_back(p.trimmed().toStdString());
    }
    QDialog::accept();
}

