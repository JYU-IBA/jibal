#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
extern "C" {
#include <jibal.h>
#include <jibal_stop.h>
}

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_calculatePushButton_clicked();

    void on_materialThicknessDoubleSpinBox_valueChanged(double arg1);

    void on_ionEnergyLineDoubleSpinBox_valueChanged(double arg1);

    void on_ionNameLineEdit_textEdited(const QString &arg1);

    void on_materialFormulaLineEdit_textEdited(const QString &arg1);

private:
    void loadGsto();
    void recalculate();
    Ui::MainWindow *ui;
    jibal *j;
    const jibal_isotope *incident;
    jibal_layer layer;
    double E;
};
#endif // MAINWINDOW_H
