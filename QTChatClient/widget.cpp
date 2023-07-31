#include "widget.h"
#include "ui_widget.h"
#include <QHostAddress>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    m_tcpSocket = new QTcpSocket(this);
    connect(m_tcpSocket,&QTcpSocket::connected,this,&Widget::onConnected);
    connect(m_tcpSocket,&QTcpSocket::disconnected,this,&Widget::onDisConnected);
    connect(m_tcpSocket,&QTcpSocket::stateChanged,this,&Widget::onStateChanged);
    connect(m_tcpSocket,&QTcpSocket::readyRead,this,&Widget::onReadyRead);
}

Widget::~Widget()
{
    delete ui;
}
void Widget::onConnected()
{

    ui->lab_state->setText("已连接到服务器");
    QString msg =  ui->lineEdit_userName->text();
    QString _msg = "#1"+msg;
    QByteArray str = _msg.toUtf8();

    qDebug()<<"write to server:"<<str;
    m_tcpSocket->write(str);

}

void Widget::onDisConnected()
{
    ui->lab_state->setText("已断开服务器");

}
void Widget::onStateChanged(QAbstractSocket::SocketState)
{

}

void Widget::onReadyRead()
{

    char buffer[1024] = {0};
    m_tcpSocket->read(buffer, 1024);
    qDebug()<<"onReadyRead()"<<buffer<<" strlen(buffer): "<<strlen(buffer);
    if( strlen(buffer) > 0)
    {
        QString showNsg = buffer;
        //qDebug()<<"onReadyRead()"<<showNsg;
        if(showNsg[0]=='#') {
            if(showNsg[1]=='3'){
                ui->lab_onlineUserNum->setText(showNsg.mid(2));
            }
            if(showNsg[1]=='2'){
                  ui->textBrowser_chatUserList->setText(showNsg.mid(2));
            }
        }else{
             ui->textBrow_chatWindow->append(showNsg);
        }


    }

    m_tcpSocket->read(buffer, 1024);

    qDebug()<<buffer;

}






void Widget::on_btn_clear_clicked()
{
    ui->textEdit_message->clear();
}

void Widget::on_btn_connectServer_clicked()
{
    QString addr = ui->lineEdit_serverIp->text();
    qint16 port = ui->lineEdit_serverPort->text().toInt();
    m_tcpSocket->connectToHost(addr,port);
    bool connected = m_tcpSocket->waitForConnected();
    qDebug()<<"connect"<<addr<<port<<connected<<endl;

    if(connected){
        ui->btn_connectServer->setEnabled(false);
        ui->btn_disConnect->setEnabled(true);
        ui->btn_send->setEnabled(true);
    }

}

void Widget::on_btn_disConnect_clicked()
{
    if(m_tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
           m_tcpSocket->disconnectFromHost();
           ui->btn_connectServer->setEnabled(true);
           ui->btn_disConnect->setEnabled(false);
           ui->btn_send->setEnabled(false);
    }
}


void Widget::on_btn_send_clicked()
{
    QString msg =ui->textEdit_message->toPlainText();
    //ui->plainTextEdit->appendPlainText("[out]"+msg);
        QByteArray str = msg.toUtf8();
        str.append('\n');
        m_tcpSocket->write(str);
       ui->textEdit_message->clear();
}





