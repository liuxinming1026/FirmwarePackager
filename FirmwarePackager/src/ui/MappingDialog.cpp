#include "MappingDialog.h"

#include <QFormLayout>
#include <QDialogButtonBox>

MappingDialog::MappingDialog(core::FileEntry& entry, QWidget* parent)
    : QDialog(parent), fileEntry(entry) {
    setWindowTitle("Edit Mapping");

    auto *layout = new QFormLayout(this);

    auto *pathEdit = new QLineEdit(QString::fromStdString(fileEntry.path.string()));
    pathEdit->setReadOnly(true);
    layout->addRow("Path", pathEdit);

    idEdit = new QLineEdit(QString::fromStdString(fileEntry.id));
    layout->addRow("ID", idEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &MappingDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &MappingDialog::reject);
    layout->addWidget(buttons);
}

void MappingDialog::accept() {
    fileEntry.id = idEdit->text().toStdString();
    QDialog::accept();
}

