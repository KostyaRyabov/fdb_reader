#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFile>
#include <QFileDialog>
#include "QDebug"

#include "mymodel.h"

#include <QMessageBox>

#include "QString"
#include "essences.h"

#include <QObject>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MyModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MyModel* m_model;
    Ui::MainWindow *ui;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void on_planes_bn_released();
    void on_company_bn_released();
    void on_status_bn_released();
    void on_hangar_bn_released();
    void on_shedule_bn_released();
    void on_way_bn_released();
    void on_cancel_bn_released();
    void on_commit_bn_released();
    void on_remove_bn_released();
    void on_save_bn_released();
    void on_add_bn_released();

    void setTitle(QString value);
    void selectionChanged();

    void on_load_bn_released();
    void on_changeDBFolder_bn_released();
};
#endif // MAINWINDOW_H
