#include "mainwindow.h"
#include "ui_mainwindow.h"


#include "QSqlRecord"
#include "QDateTime"

#include <QTableView>
#include "essences.h"

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
    m_model->saveDirPath(Tools::InitDB());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_planes_bn_released()
{
    m_model->select("planes");
}

void MainWindow::on_company_bn_released()
{
    m_model->select("company");
}

void MainWindow::on_status_bn_released()
{
    m_model->select("status");
}

void MainWindow::on_hangar_bn_released()
{
    m_model->select("hangar");
}

void MainWindow::on_shedule_bn_released()
{
    m_model->select("shedule");
}

void MainWindow::on_way_bn_released()
{
    m_model->select("way");
}

void MainWindow::on_cancel_bn_released()
{
    m_model->reloadTable();
}

void MainWindow::on_commit_bn_released()
{
    if (m_model->UpdateData()) m_model->reloadTable();
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
            ui->remove_bn->setText("remove");
            ui->save_bn->setText("save");
            ui->save_bn->setEnabled(false);
        }
    }
}

void MainWindow::on_changeDBFolder_bn_released()
{
    QString dirPath = QFileDialog::getExistingDirectory(0,"set database folder","") + "/";

    if (dirPath == "/"){
        QDir().mkdir("database");
        dirPath = QDir::currentPath()+"/database/";
    }

    for (auto &table : QStringList{"planes","company","status","hangar","shedule","way"}){
        if (!QFile(dirPath + table + fileFormat).exists())
            Tools::makeEmptyTableFile(table, dirPath);
    }

    m_model->saveDirPath(dirPath);
}
