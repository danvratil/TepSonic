#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include <QTreeWidgetItem>
#include <QDebug>

PreferencesDialog::PreferencesDialog(QSettings& settings, QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::PreferencesDialog)
{
    _ui->setupUi(this);
    _settings = &settings;

    _ui->enableCollectionsCheckbox->setChecked(_settings->value("Collections/EnableCollections",true).toBool());
    _ui->autoRebuildCheckbox->setChecked(_settings->value("Collections/AutoRebuildAfterStart",true).toBool());
    _ui->rememberLastSessionCheckbox->setChecked(_settings->value("Preferences/RestoreSession",true).toBool());

}

PreferencesDialog::~PreferencesDialog()
{
    delete _ui;
}

void PreferencesDialog::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        _ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void PreferencesDialog::on_buttonBox_accepted()
{
    _settings->setValue("Collections/EnableCollections",_ui->enableCollectionsCheckbox->isChecked());
    _settings->setValue("Collections/AutoRebuildAfterStart",_ui->autoRebuildCheckbox->isChecked());
    _settings->setValue("Preferences/RestoreSession",_ui->rememberLastSessionCheckbox->isChecked());

    this->close();
}

void PreferencesDialog::on_buttonBox_rejected()
{
    this->close();
}

