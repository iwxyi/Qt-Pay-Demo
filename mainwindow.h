#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QFileInfo>
#include <QTimer>
#include "netutil.h"

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
    void on_pushButton_clicked();

    void slotQueryIsPaid();

private:
    Ui::MainWindow *ui;

    const QString server; // 后台服务网址
    QString payjs_order_id;
    QTimer* query_timer;
};
#endif // MAINWINDOW_H
