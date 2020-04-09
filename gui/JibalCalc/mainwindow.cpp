#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    jibal = jibal_init(nullptr);
    incident = nullptr;
    layer.material = nullptr;
    layer.thickness = 2000.0*C_TFU;
    E = 2000.0*C_KEV;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_calculatePushButton_clicked()
{

  ui->energyOutputDoubleSpinBox->setValue(jibal_layer_energy_loss(jibal.gsto, incident, &layer, E, -1.0)/C_KEV);
}

void MainWindow::on_materialThicknessDoubleSpinBox_valueChanged(double arg1)
{
    layer.thickness = arg1*C_TFU;
    recalculate();
}

void MainWindow::on_ionEnergyLineDoubleSpinBox_valueChanged(double arg1)
{
    E = arg1*C_KEV;
    recalculate();
}

void MainWindow::on_ionNameLineEdit_textEdited(const QString &arg1)
{
    incident =jibal_isotope_find(jibal.isotopes, arg1.toStdString().c_str(), 0, 0);
    loadGsto();
    recalculate();
}

void MainWindow::on_materialFormulaLineEdit_textEdited(const QString &arg1)
{
    if(layer.material)
        jibal_material_free(layer.material);
    layer.material=jibal_material_create(jibal.elements, arg1.toStdString().c_str());
    if(layer.material) {
        ui->tableWidget->setRowCount(layer.material->n_elements);
        for(int i = 0; i < layer.material->n_elements; ++i) {
        QTableWidgetItem *newItem = new QTableWidgetItem(tr("%1").arg(layer.material->elements[i].name));
        ui->tableWidget->setItem(i, 0, newItem);
        newItem = new QTableWidgetItem(tr("%1").arg(layer.material->concs[i]*100.0));
        ui->tableWidget->setItem(i, 1, newItem);
        }
    } else {
        ui->tableWidget->setRowCount(0);
    }
    loadGsto();
    recalculate();
}

void MainWindow::loadGsto()
{
    if(!layer.material || !incident)
        return;
    jibal_gsto_auto_assign_material(jibal.gsto, incident, layer.material);
    jibal_gsto_load_all(jibal.gsto); /* TODO: calling this might not be necessary always. */
}

void MainWindow::recalculate()
{
    if(!layer.material || !incident)
        return;
    ui->energyOutputDoubleSpinBox->setValue(jibal_layer_energy_loss(jibal.gsto, incident, &layer, E, -1.0)/C_KEV);
}
