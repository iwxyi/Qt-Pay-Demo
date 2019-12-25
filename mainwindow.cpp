#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      server("http://writerfly.cn/server/pay/")
{
    ui->setupUi(this);

    query_timer = new QTimer(this);
    query_timer->setInterval(1000);
    connect(query_timer, SIGNAL(timeout()), this, SLOT(slotQueryIsPaid()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString param = "user_id=12345678&total_fee=0.03&purchase_type=1&attach=nothing";
    const QString url = server + "wxpay_pay.php?" + param;

    connect(new NetUtil(url), &NetUtil::finished, this, [=](QString s){
        qDebug() << "接口结果：" << s;
        /* Array (
             [code_url] => weixin://wxpay/bizpayurl?pr=OehN4er
             [out_trade_no] => 123456781577192089
             [payjs_order_id] => 2019122420544900318957784
             [qrcode] => https://payjs.cn/qrcode/d2VpeGluOi8vd3hwYXkvYml6cGF5dXJsP3ByPU9laE40ZXI=
             [return_code] => 1
             [return_msg] => SUCCESS
             [total_fee] => 3
             [sign] => E133A9A6FC7CA4EDDAF5F6081B3A5D65
           ) */

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
            ui->label->setText("");
            ui->label->setScaledContents(true);
            ui->label->setPixmap(QPixmap(path));
            net->deleteLater();
        });
        this->payjs_order_id = payjs_order_id;

        // 不断查询，直到支付成功
        query_timer->start();
    });
}

/**
 * 定时查询订单
 */
void MainWindow::slotQueryIsPaid()
{
    if (payjs_order_id.isEmpty())
        return ;

    const QString url = server + "pay_query.php?order_id=" + payjs_order_id;
    connect(new NetUtil(url), &NetUtil::finished, this, [=](QString s){
        qDebug() << "查询结果：" << s;
        /* 已支付
           Array (
            [attach] => test_order_attach
            [mchid] => 1571300721
            [openid] => oLw30sh9bE0nj54xTOC157SQryNg
            [out_trade_no] => 1577169826
            [paid] => 1
            [paid_time] => 2019-12-24 14:45:35
            [payjs_order_id] => 2019122414434600295140484
            [return_code] => 1
            [status] => 1
            [total_fee] => 2
            [transaction_id] => 4200000471201912249642685702
            [sign] => 9734B26FA66559CF9B0ED16F2ADF676E
        ) */

        /* 等待扫码/扫码未支付
         * Array (
         * [attach] => nothing<USER_ID>12345678</USER_ID><PURCHASE_TYPE>1</PURCHASE_TYPE>
         * [mchid] => 1571300721
         * [openid] => 0
         * [out_trade_no] => 123456781577238473
         * [paid] => 0
         * [paid_time] => 0
         * [payjs_order_id] => 2019122509475400770614789
         * [return_code] => 1
         * [status] => 0
         * [total_fee] => 3
         * [transaction_id] => 0
         * [sign] => 0A1B24C483D0407DA9D2F33FD9FA6E95 )
         */

        /* 不存在订单
         * Array ( [return_code] => 0 [status] => 0 [msg] => 订单号错误 )
         */

        QString return_code = NetUtil::extractOne(s, "\\[return_code\\]\\s*=>\\s*(\\S+)\\s*");
        if (return_code == "0") // 不存在订单
            return ;

        // 查看 paid 是不是 1，未支付时为0
        QString paid = NetUtil::extractOne(s, "\\[paid\\]\\s*=>\\s*(\\S+)\\s*");
        if (paid == "1")
        {
            query_timer->stop();
            ui->label->setPixmap(QPixmap());
            ui->label->setText("支付成功");
            payjs_order_id = "";
        }
    });
}
