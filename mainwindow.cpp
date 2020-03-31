#include "mainwindow.h"
#include "ui_mainwindow.h"


#include "QSqlRecord"
#include "QDateTime"

#include <QTableView>
#include "essences.h"

#include <QSqlField>
#include <QDebug>
#include "mymodel.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(" ");

    m_model = new MyModel(this);

    QObject::connect(m_model,
                     SIGNAL(setTitle(QString)),
                     this,
                     SLOT(setTitle(QString)));

    ui->tableView->setModel(m_model);

    QObject::connect(ui->tableView->selectionModel(),
                 SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                 this,
                 SLOT(selectionChanged()));

    ui->tableView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_model->bindView(ui);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_planes_bn_released()
{
    m_model->select<essences::planes>();
}

void MainWindow::on_company_bn_released()
{
    m_model->select<essences::company>();
}

void MainWindow::on_status_bn_released()
{
    m_model->select<essences::status>();
}

void MainWindow::on_hangar_bn_released()
{
    m_model->select<essences::hangar>();
}

void MainWindow::on_shedule_bn_released()
{
    m_model->select<essences::shedule>();
}

void MainWindow::on_way_bn_released()
{
    m_model->select<essences::way>();
}

void MainWindow::on_cancel_bn_released()
{
    m_model->reloadTable();
}

void MainWindow::on_commit_bn_released()
{
    if (m_model->UpdateData() && m_model->InsertData()) m_model->reloadTable();
    else {
        m_model->clear();
        setWindowTitle("");
    }
}

void MainWindow::on_remove_bn_released()
{
    if (m_model->RemoveData()) m_model->reloadTable();
    else {
        m_model->clear();
        setWindowTitle("");
    }
}

void MainWindow::on_load_bn_released()
{
    m_model->loadData();
}

void MainWindow::on_save_bn_released()
{
    m_model->saveData();
}

void MainWindow::on_add_bn_released()
{
    m_model->AddEmptyRow();
}


void MainWindow::setTitle(QString newValue){
    setWindowTitle(newValue);
}

void MainWindow::selectionChanged(){
    bool enable = !ui->tableView->selectionModel()->selectedRows().isEmpty();
    ui->remove_bn->setEnabled(enable);

    if (enable){
        auto size = QVariant(ui->tableView->selectionModel()->selectedRows().count()).toString();
        ui->remove_bn->setText("remove ("+size+")");
        ui->save_bn->setText("save ("+size+")");
        ui->save_bn->setEnabled(true);
    }else{
        if (ui->tableView->model()->rowCount() > 0){
            ui->save_bn->setText("save (all)");
            ui->save_bn->setEnabled(true);
        }else{
            ui->save_bn->setText("save");
            ui->save_bn->setEnabled(false);
        }
    }
}
