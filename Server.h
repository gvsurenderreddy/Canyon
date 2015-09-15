#ifndef Server_h__
#define Server_h__

#include <QObject>
#include <QString>
#include <QTimer>
#include "websockets/QWsSocket.h"

class Canyon;

#define SVC_URL "ws://svc.canyonvpn.com"

class Server : public QObject
{
	Q_OBJECT

public:
	Server(Canyon* cn);
	void Reset();

	void Login(QString u, QString p);
	void AddPeer(QString peer);
	void RemovePeer(QString peer);
	void SendMessage(QString peer, QString message);
	void AcceptRequest(QString peer);
	void RejectRequest(QString peer);
	void UpdateCAS(QString cas);

	friend class Canyon;

public slots:
	void Ping(); // timerPing
	void OnTimerFinished(); // timerLoading
	void OnSocketConnected();
	void OnSocketDisconnected();
	void OnSocketResponse(QString message);
	void OnSocketError(QAbstractSocket::SocketError socketError);

signals:
	void OnServerMessage(QString message);

private:
	QString username;
	QString password;
	QString version;
	QtWebsocket::QWsSocket* wsSocket;
	QTimer timerPing;
	QTimer timerLoading;
	Canyon* canyon;
};
#endif // Server_h__

