#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:


    void on_btn_clear_clicked();

    void on_btn_connectServer_clicked();
    void onConnected();

       void onDisConnected();

       void onStateChanged(QAbstractSocket::SocketState);

       void onReadyRead();


       void on_btn_send_clicked();

       void on_btn_disConnect_clicked();

private:
    Ui::Widget *ui;

    QTcpSocket *m_tcpSocket = nullptr;
};

#endif // WIDGET_H
