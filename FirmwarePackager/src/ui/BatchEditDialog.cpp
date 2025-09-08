#include "BatchEditDialog.h"

#include <QFormLayout>
#include <QDialogButtonBox>

BatchEditDialog::BatchEditDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Batch Edit");
    auto *layout = new QFormLayout(this);

    prefixEdit = new QLineEdit;
    layout->addRow("Dest Prefix", prefixEdit);

    modeEdit = new QLineEdit;
    layout->addRow("Mode", modeEdit);

    ownerEdit = new QLineEdit;
    layout->addRow("Owner", ownerEdit);

    groupEdit = new QLineEdit;
    layout->addRow("Group", groupEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &BatchEditDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &BatchEditDialog::reject);
    layout->addWidget(buttons);
}

QString BatchEditDialog::prefix() const { return prefixEdit->text(); }
QString BatchEditDialog::mode() const { return modeEdit->text(); }
QString BatchEditDialog::owner() const { return ownerEdit->text(); }
QString BatchEditDialog::group() const { return groupEdit->text(); }

