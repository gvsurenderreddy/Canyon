#ifndef Peer_h__
#define Peer_h__

#include <QObject>
#include <QDialog>
#include <QTcpSocket>
#include "ui_itempeer.h"
#include "ui_chatform.h"

class DialogChat : public QDialog
{
	Q_OBJECT

public:
	DialogChat(QWidget *parent = 0, Qt::WFlags flags = 0);
	
	Ui::FormChat ui;
};

enum E_PEER_STATUS
{
	STATUS_SERVER_ONLINE = 1,
	STATUS_P2P_ONLINE = 2,
	STATUS_DUKTO_ONLINE = 4,
};

class Peer : public QWidget
{
	Q_OBJECT

public:
	Peer(QWidget *parent = 0, QString loc = "");
	~Peer();
	void SetStatus(int stat, bool neg = false);	
	void SetName(QString _name);
	void ReceiveMessage(QString message);
	void LoadChatHistory();
	void SaveChatHistory();

public slots:
	void OnSendFile();
	void OnPressEnter();
	void OnContextMenu(const QPoint& pos);
	void showContextMenu(const QPoint&);
	void OnP2PReceive();
	void OnP2PError(QAbstractSocket::SocketError error);

signals:
	void MessageSend(Peer* peer, QString message);
	void FileSend(Peer* peer, QStringList files);
	void Remove(Peer* peer);

public:
	DialogChat* dialogChat;
	QTcpSocket* tcpSocket;
	Ui::ItemPeer uiItem;
	QStringList chatLines;
	QString location;

	// QString cas;
	// QString fingerprint;
	QString name;
	QString uid;
	QString ip;
	qint16 port;
	int status;
};

#endif // Peer_h__
