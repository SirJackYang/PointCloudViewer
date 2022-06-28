#ifndef POINTPICKERDLG_H
#define POINTPICKERDLG_H
#pragma execution_character_set("UTF-8")
#include <QDialog>
#include <QFileDialog>
#include <QString>
#include <QCloseEvent>
#include <QFile>
#include <vector>
#include "CommeTools.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PointPickerDlg; }
QT_END_NAMESPACE

class PointPickerDlg : public QDialog
{
    Q_OBJECT

public:
    PointPickerDlg(QWidget *parent = nullptr);
    ~PointPickerDlg();

private slots:
    void on_toolButton_Save_clicked();

    void on_toolButton_F5_clicked();

private:
    Ui::PointPickerDlg *ui;
    std::vector<QString> m_points;
    bool m_saved;
    //! Associated 3D label
    //!
signals:
    void DeleteLastPoint();
    void CloseMe();

public:
    void Add2Table(QString name, double x, double y, double z);

protected:
    void closeEvent(QCloseEvent *event);
};
#endif // POINTPICKERDLG_H
