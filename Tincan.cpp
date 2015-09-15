#include "Tincan.h"
#include "MainForm.h"
#include "Peer.h"
#include <QCryptographicHash>
#include <QTime>


Tincan::Tincan(void)
{
	QTime time = QTime::currentTime();
	qsrand((uint)time.msec());

	listStun.push_back("stun.l.google.com:19302");
	listStun.push_back("stun1.l.google.com:19302");
	listStun.push_back("stun2.l.google.com:19302");
	listStun.push_back("stun3.l.google.com:19302");
	listStun.push_back("stun4.l.google.com:19302");


	socket = new QUdpSocket(this);
	socket->bind(QHostAddress::LocalHost, 5801);
	connect(socket, SIGNAL(readyRead()), this, SLOT(ReadPendingDatagrams()));


	SendCommand("\x2\x1{\"logging\": 2, \"m\": \"set_logging\"}");
	SendCommand("\x2\x1{\"trim_enabled\": false, \"m\": \"set_trimpolicy\"}");
	SendCommand("\x2\x1{\"translate\": 1, \"m\": \"set_translation\"}");
	SendCommand("\x2\x1{\"ip\": \"::\", \"m\": \"set_cb_endpoint\", \"port\": 62608}");

}


Tincan::~Tincan(void)
{
}

void Tincan::ReadPendingDatagrams()
{
	while (socket->hasPendingDatagrams()) 
	{
		QByteArray datagram;
		datagram.resize(socket->pendingDatagramSize());
		QHostAddress sender;
		quint16 senderPort;

		socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

		QString result(datagram);

		if(result.startsWith("cas"))
		{
			emit CasGenerated(result);
		}
		else if(result.startsWith("online"))
		{
			QString uid = result.section(" ", 1);

			emit PeerStatus(uid, true);
		}
		else if(result.startsWith("offline"))
		{
			QString uid = result.section(" ", 1);

			emit PeerStatus(uid, false);
		}
	}
}

void Tincan::SendCommand(QString cmd)
{
	socket->writeDatagram(cmd.toAscii(), QHostAddress::LocalHost, 5800);
}

QString Tincan::CalculateIPV6(QString uid)
{
	QString base = MainForm::settings.value("ipv6Base", "fd50:0dbc:41f2:4a3c").toString();

	for(int i = 0; i < 16; i += 4)
	{
		base += ":" + uid.mid(i, 4);
	}
	return base;
}


void Tincan::Login(QString uid)
{
	SendCommand("\x2\x1{\"ip4_mask\": 24, \"ip6_mask\": 64, \"uid\": \"" 
		+ uid + "\", \"m\": \"set_local_ip\", \"subnet_mask\": 32, \"ip6\": \"" + CalculateIPV6(uid) 
		+ "\", \"ip4\": \"" + MainForm::settings.value("ipv4Base", "172.31.0.100").toString() + "\"}");

	//SendCommand("\x2\x1{\"m\": \"get_fpr\"}");
}

void Tincan::SetPeerCasAndIP(Peer* peer, QString cas, QString fpr)
{
	SendCommand("\x2\x1{\"uid\": \"" + peer->uid + "\", \"cas\": \"" + cas + "\", \"m\": \"create_link\", \"turn\": \"\", \"overlay_id\": 0, \"sec\": false, \"fpr\": \"" 
		+ fpr + "\", \"turn_user\": \"\", \"turn_pass\": \"\", \"stun\": \"" + listStun[qrand() % listStun.count()] + "\"}");


	SendCommand("\x2\x1{\"ip6\": \"" + CalculateIPV6(peer->uid) + "\", \"m\": \"set_remote_ip\", \"uid\": \"" + peer->uid + 
		"\", \"ip4\": \"" + peer->ip + "\"}");
}

void Tincan::TrimLink(Peer* peer)
{
	SendCommand("\x2\x1{\"m\": \"trim_link\", \"uid\": \"" + peer->uid + "\"}");
}
