#ifndef Canyon_h__
#define Canyon_h__

#include <QObject>
#include <QWidget>
#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkSession>
#include "MainForm.h"
#include "Server.h"
#include "Tincan.h"
#include "Dukto.h"
#include "Peer.h"
#include "Notification.h"

#define CANYON_VERSION "107"

class Canyon : public QObject
{
	Q_OBJECT

public:
	Canyon();
	~Canyon(void);

	void Reset();
	void Login();
	void Logout();
	void AddPeer(QString name);
	QString GeneratePeerIpv4(Peer* peer);
	QString ComputeSHA1(QString input);

	QString DecodeBase64(QString string);

public slots:
	// Signals from Notification
	void OnNotificationAction(bool accept);

	// Signals from Server
	void OnServerMessage(QString message);

	// Signals from Peer
	void OnDeletePeer(Peer* peer);
	void OnMessageSend(Peer* peer, QString message);
	void OnFileSend(Peer* peer, QStringList files);

	// Signals from Tincan
	void OnCasGenerated(QString cas);
	void OnPeerStatus(QString uid, bool online);
	void TrimTick();
	void ReconnectTick();

	// Signals from TcpServer
	void NewP2PConnection();


public:
	MainForm form;
	Server server;
	Tincan tincan;
	Dukto dukto;
	QTimer timerTrim;
	QTimer timerReconnect;


	QTcpServer* tcpServer;

	QList<Peer*> peers;
	QList<Notification*> notifications;
};
#endif // Canyon_h__

