#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QtGui/QDialog>

#include <QDirModel>
#include <QTreeWidgetItem>
#include <QSettings>

namespace Ui {
    class PreferencesDialog;
}

class PreferencesDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(PreferencesDialog)
public:
    PreferencesDialog(QSettings* settings, QWidget *parent = 0);
    virtual ~PreferencesDialog();

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::PreferencesDialog *_ui;

    QSettings *_settings;

private slots:


private slots:
    void on_addPathButton_clicked();
    void on_removePathButton_clicked();
    void on_collectionsStorageEngine_combo_currentIndexChanged(QString newIndex);
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();
};

#endif // PREFERENCESDIALOG_H
