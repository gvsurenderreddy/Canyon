#ifndef Tincan_h__
#define Tincan_h__

#include <QObject>
#include <QUdpSocket>

class Peer;

class Tincan : public QObject
{
	Q_OBJECT
public:
	Tincan(void);
	~Tincan(void);

	void Login(QString uid);
	void SetPeerCasAndIP(Peer* peer, QString cas, QString fpr);
	void TrimLink(Peer* peer);

public slots:
	void ReadPendingDatagrams();

signals:
	void PeerStatus(QString uid, bool online);
	void CasGenerated(QString cas);

private:
	void SendCommand(QString cmd);
	QString CalculateIPV6(QString uid);

	QList<QString> listStun;
	QUdpSocket* socket;
	QString commandData;
};

#endif // Tincan_h__
