#include "Server.h"

#include <QCryptographicHash>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSsl>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QDesktopServices>
#include "Canyon.h"
#include "Peer.h"

Server::Server(Canyon* cn)
	: timerPing(this), canyon(cn)
{
	version = CANYON_VERSION;//QCryptographicHash::hash(QString(__DATE__).toUtf8(), QCryptographicHash::Sha1).toHex();
	wsSocket = new QtWebsocket::QWsSocket(this, NULL, QtWebsocket::WS_V13);

	// Load SSL config
	//QSslConfiguration defaultSSLConfig = QSslConfiguration::defaultConfiguration();
	//QList<QSslCertificate> certificates;
	//QFile cert;
	//cert.setFileName(":/MainForm/Resources/localhost.crt");
	//if(cert.open(QIODevice::ReadOnly))
	//{
	//	QSslCertificate certificate(&cert, QSsl::Pem);
	//	certificates.append(certificate);
	//}
	//defaultSSLConfig.setCaCertificates(certificates);
	//QSslConfiguration::setDefaultConfiguration(defaultSSLConfig);

	connect(&timerPing, SIGNAL(timeout()), this, SLOT(Ping()));
	connect(&timerLoading, SIGNAL(timeout()), this, SLOT(OnTimerFinished()));

	connect(wsSocket, SIGNAL(frameReceived(QString)), this, SLOT(OnSocketResponse(QString)));
	connect(wsSocket, SIGNAL(connected()), this, SLOT(OnSocketConnected()));
	connect(wsSocket, SIGNAL(disconnected()), this, SLOT(OnSocketDisconnected()));
	connect(wsSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(OnSocketError(QAbstractSocket::SocketError)));
}

void Server::Login(QString u, QString p)
{
	username = u;
	password = QCryptographicHash::hash(p.toUtf8(), QCryptographicHash::Sha1).toHex();

	wsSocket->connectToHost(SVC_URL, 80);
}

void Server::Ping()
{
	wsSocket->write(QString(1));
}

void Server::AddPeer(QString peer)
{
	wsSocket->write((QString) "add " + peer);
}

void Server::RemovePeer(QString peer)
{
	wsSocket->write((QString) "remove " + peer);
}

void Server::SendMessage(QString peer, QString message)
{
	wsSocket->write((QString) "message " + peer + " " + message);
}

void Server::AcceptRequest(QString peer)
{
	wsSocket->write((QString) "accept " + peer);
}

void Server::RejectRequest(QString peer)
{
	wsSocket->write((QString) "reject " + peer);
}

void Server::UpdateCAS(QString cas)
{
	wsSocket->write((QString) "cas " + cas);
}

void Server::Reset()
{
	wsSocket->disconnectFromHost();

	username.clear();
	password.clear();

	timerLoading.stop();
	timerPing.stop();
}

void Server::OnSocketResponse(QString message)
{
	emit OnServerMessage(message);
}

void Server::OnSocketConnected()
{
	wsSocket->write((QString) "version " + version);
	wsSocket->write((QString) "login " + username + " " + password);
}

void Server::OnSocketError(QAbstractSocket::SocketError socketError)
{
	emit OnServerMessage("error");
}

void Server::OnTimerFinished()
{
	timerLoading.stop();
	emit OnServerMessage("loadFinished");
}

void Server::OnSocketDisconnected()
{
	//emit OnServerMessage("error");
}
