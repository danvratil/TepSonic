#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include <QTreeWidgetItem>

PreferencesDialog::PreferencesDialog(QSettings& settings, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::PreferencesDialog)
{
    m_ui->setupUi(this);
    m_settings = &settings;

    m_ui->enableCollectionsCheckbox->setChecked(m_settings->value("Collections/EnableCollections",true).toBool());
    m_ui->autoRebuildCheckbox->setChecked(m_settings->value("Collections/AutoRebuildAfterStart",true).toBool());
    m_ui->rememberLastSessionCheckbox->setChecked(m_settings->value("Preferences/RestoreSession",true).toBool());
}

PreferencesDialog::~PreferencesDialog()
{
    delete m_ui;
}

void PreferencesDialog::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void PreferencesDialog::on_buttonBox_accepted()
{
    m_settings->setValue("Collections/EnableCollections",m_ui->enableCollectionsCheckbox->isChecked());
    m_settings->setValue("Collections/AutoRebuildAfterStart",m_ui->autoRebuildCheckbox->isChecked());
    m_settings->setValue("Preferences/RestoreSession",m_ui->rememberLastSessionCheckbox->isChecked());

    this->close();
}

void PreferencesDialog::on_buttonBox_rejected()
{
    this->close();
}
