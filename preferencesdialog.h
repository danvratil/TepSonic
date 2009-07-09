#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QtGui/QDialog>

#include <QTreeWidgetItem>
#include <QSettings>

namespace Ui {
    class PreferencesDialog;
}

class PreferencesDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(PreferencesDialog)
public:
    PreferencesDialog(QSettings& settings, QWidget *parent = 0);
    virtual ~PreferencesDialog();

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::PreferencesDialog *m_ui;

    QSettings *m_settings;

private slots:


private slots:
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();
};

#endif // PREFERENCESDIALOG_H
