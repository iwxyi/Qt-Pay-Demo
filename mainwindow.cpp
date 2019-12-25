#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString param = "user_id=12345678&total_fee=0.03&purchase_type=1&attach=nothing";
    const QString url = "http://writerfly.cn/server/pay/wxpay_pay.php?" + param;

//    Array\n(\n    [code_url] => weixin://wxpay/bizpayurl?pr=OehN4er\n    [out_trade_no] => 123456781577192089\n    [payjs_order_id] => 2019122420544900318957784\n    [qrcode] => https://payjs.cn/qrcode/d2VpeGluOi8vd3hwYXkvYml6cGF5dXJsP3ByPU9laE40ZXI=\n    [return_code] => 1\n    [return_msg] => SUCCESS\n    [total_fee] => 3\n    [sign] => E133A9A6FC7CA4EDDAF5F6081B3A5D65\n)\n"
    connect(new NetUtil(url), &NetUtil::finished, this, [=](QString s){
        qDebug() << "接口结果：" << s;
        QString payjs_order_id = NetUtil::extractOne(s, "\\[payjs_order_id\\]\\s*=>\\s*(\\S+)\\s*");
        QString qrcode = NetUtil::extractOne(s, "\\[qrcode\\]\\s*=>\\s*(\\S+)\\s*");
        QString return_code = NetUtil::extractOne(s, "\\[return_code\\]\\s*=>\\s*(\\S+)\\s*");

        if (return_code.trimmed() != "1")
        {
            QMessageBox::warning(this, "错误", "获取支付链接出错\n返回码：" + return_code);
            return ;
        }

        /**
         * SSL 出错的，需要安装 OpenSSL，拷贝安装目录的 ssleay32.dll 和 libeay32.dll 至运行目录或者 .../MinGW730_32/opt/bin/ 下
         */

        const QString path = QApplication::applicationDirPath() + "/wxpay.png";
        NetUtil* net = new NetUtil;
        net->download(qrcode, path);
        connect(net, &NetUtil::finished, this, [=](QString path){
            qDebug() << "二维码路径：" << path;
            ui->label->setScaledContents(true);
            ui->label->setPixmap(QPixmap(path));
            net->deleteLater();
        });
    });
}
