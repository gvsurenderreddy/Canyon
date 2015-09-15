#include "Canyon.h"
#include <QDir>
#include <QApplication>
#include <QProcess>
#include <windows.h>
#include <shellapi.h>

#undef SendMessage

Canyon::Canyon()
	: timerTrim(this), server(this), tcpServer(0)
{
	form.canyon = this;

	QStringList args = QApplication::arguments();

	if(args.count() > 1 && args[1] == "/background")
	{
		form.hide();
	}
	else
	{
		form.show();
	}

	if(form.settings.value("autoLogin", false).toBool())
	{
		form.OnBtnLogin();
	}

	QDir dir;
	dir.mkpath(form.settings.value("chatLogsDirectory", QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "\\Canyon\\Chat Logs").toString());


	connect(&tincan, SIGNAL(CasGenerated(QString)), this, SLOT(OnCasGenerated(QString)));
	connect(&tincan, SIGNAL(PeerStatus(QString, bool)), this, SLOT(OnPeerStatus(QString, bool)));

	connect(&server, SIGNAL(OnServerMessage(QString)), this, SLOT(OnServerMessage(QString)));
	connect(&timerTrim, SIGNAL(timeout()), this, SLOT(TrimTick()));
	connect(&timerReconnect, SIGNAL(timeout()), this, SLOT(ReconnectTick()));


	tcpServer = new QTcpServer(this);
	tcpServer->listen(QHostAddress(MainForm::settings.value("ipv4Base", "172.31.0.100").toString()), 4644);
	connect(tcpServer, SIGNAL(newConnection()), this, SLOT(NewP2PConnection()));

}

Canyon::~Canyon(void)
{
	Reset();
}

void Canyon::OnNotificationAction(bool accept)
{
	Notification* not = dynamic_cast<Notification*>(QObject::sender());

	if(not->m_type == NOTIFICATION_PEER)
	{
		if(accept)
		{
			server.AcceptRequest(not->m_data);
			AddPeer(not->m_data);
		}
		else
		{
			server.RejectRequest(not->m_data);
		}
	}
	else if(not->m_type == NOTIFICATION_DOWNLOAD)
	{
		// dukto.rejectdownload
	}
	else if(not->m_type == NOTIFICATION_PROGRESS)
	{
		// dukto.canceldownload
	}

	form.ui.verticalLayout_8->removeWidget(not);
	notifications.removeOne(not);
	delete not;
}

void Canyon::Reset()
{
	server.Reset();

	Q_FOREACH(Peer* peer, peers)
	{
		form.ui.verticalLayout_8->removeWidget(peer);
		tincan.TrimLink(peer);
		delete peer;
	}
	peers.clear();

	Q_FOREACH(Notification* not, notifications)
	{
		form.ui.verticalLayout_8->removeWidget(not);
		delete not;
	}
	notifications.clear();
}

void Canyon::Login()
{
	server.Login(form.ui.editUsername->text(), form.ui.editPassword->text());
}

void Canyon::OnDeletePeer(Peer* peer)
{
	server.RemovePeer(peer->name);

	form.ui.verticalLayout_8->removeWidget(peer);
	tincan.TrimLink(peer);
	delete peer;
	peers.removeOne(peer);
}

void Canyon::OnMessageSend(Peer* peer, QString message)
{
	 if(peer->status & STATUS_DUKTO_ONLINE)
	 {
		 //dukto.sendmessage(peer, message);
	 }
	 else
	 {
		 server.SendMessage(peer->name, message);
	 }
}

void Canyon::AddPeer(QString name)
{
	Peer* peer = new Peer(&form, form.settings.value("chatLogsDirectory", 
		QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "\\Canyon\\Chat Logs").toString()
		+ "\\" + form.ui.editUsername->text() + "." + name);
	peers.push_back(peer);
	peer->SetName(name);
	peer->uid = ComputeSHA1(name);
	peer->ip = GeneratePeerIpv4(peer);
	peer->SetStatus(0);

	connect(peer, SIGNAL(Remove(Peer*)), this, SLOT(OnDeletePeer(Peer*)));
	connect(peer, SIGNAL(FileSend(Peer*, QStringList)), this, SLOT(OnFileSend(Peer*, QStringList)));
	connect(peer, SIGNAL(MessageSend(Peer*, QString)), this, SLOT(OnMessageSend(Peer*, QString)));

	form.ui.verticalLayout_8->insertWidget(1, peer, 1);
}

void Canyon::OnFileSend(Peer* peer, QStringList files)
{
	// duto.sendfile(peer, files)
}

void Canyon::OnCasGenerated(QString cas)
{
	QString uid = cas.section(" ", 1, 1);
	QString sc = cas.section(" ", 2);

	Q_FOREACH(Peer* peer, peers)
	{
		if(peer->uid == uid && !(peer->status & STATUS_P2P_ONLINE) && (peer->status & STATUS_SERVER_ONLINE))
		{
			server.UpdateCAS(peer->name + " " + sc);
			break;
		}
	}

}

QString Canyon::GeneratePeerIpv4(Peer* peer)
{
	QString ipBase = MainForm::settings.value("ipv4Base", "172.31.0.100").toString();
	int finalOctet = ipBase.section('.', 3).toInt() + 1 + peers.indexOf(peer);
	return ipBase.section('.', 0, 2) + "." + QString::number(finalOctet);
}

QString Canyon::ComputeSHA1(QString input)
{
	QByteArray hash = QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Sha1);
	return hash.toHex();
}

void Canyon::OnPeerStatus(QString uid, bool online)
{
	Q_FOREACH(Peer* peer, peers)
	{
		if(peer->uid == uid)
		{
			peer->SetStatus(STATUS_P2P_ONLINE, online ? false : true);

			if(!online)
			{
				tincan.SetPeerCasAndIP(peer, "", "");
			}

			break;
		}
	}
}

void Canyon::TrimTick()
{
	Q_FOREACH(Peer* peer, peers)
	{
		if(!(peer->status & STATUS_P2P_ONLINE))
		{
			tincan.TrimLink(peer);
			tincan.SetPeerCasAndIP(peer, "", "");
		}
	}
}

void Canyon::Logout()
{
	Reset();
}

void Canyon::OnServerMessage(QString message)
{
	if(message == "error")
	{
		form.ui.labelError->setText("Unable to connect to server.\nCheck status at canyonvpn.com\nReconnecting in 15s...");
		form.ui.labelError->show();
		form.ui.stackedWidget->slideInIdx(0, SlidingStackedWidget::LEFT2RIGHT);

		Reset();

		timerReconnect.start(15000);
		timerTrim.stop();
	}
	else if(message == "loadFinished")
	{
		form.ui.stackedWidget->slideInIdx(2, SlidingStackedWidget::RIGHT2LEFT);
	}
	else if(message.startsWith("friends"))
	{
		server.timerLoading.start(3000);
		server.timerPing.start(45000); // 45 secs enough?

		tincan.Login(ComputeSHA1(server.username));

		QStringList friends = message.split(' ', QString::SkipEmptyParts);

		for(uint i = 1; i < friends.size(); ++i)
		{
			QString name = friends[i];

			AddPeer(name);
		}


		timerTrim.start(30000);
	}
	else if(message.startsWith("login"))
	{
		form.ui.labelError->setText("The username or password entered are incorrect.");
		form.ui.labelError->show();
		form.ui.stackedWidget->slideInIdx(0, SlidingStackedWidget::BOTTOM2TOP);

		Reset();
	}
	else if(message.startsWith("cas"))
	{
		QString name = message.section(' ', 1, 1);
		QString temp = message.section(' ', 2);

		if(temp == "")
			return;

		QString fingerprint = temp.section(' ', 0, 0);
		QString cas = temp.section(' ', 1);

		Q_FOREACH(Peer* peer, peers)
		{
			if(peer->name == name && !(peer->status & STATUS_P2P_ONLINE)  /*&& peer->cas != cas*/)
			{
				tincan.SetPeerCasAndIP(peer, cas, fingerprint);

				break;
			}
		}
	}
	else if(message.startsWith("ofmsg"))
	{
		QStringList list = message.split(' ', QString::SkipEmptyParts);
		list.pop_front();

		for(int i = 0; i < list.size(); i += 3)
		{
			QString sender = list[i];
			QString message = DecodeBase64(list[i + 2]);

			Q_FOREACH(Peer* peer, peers)
			{
				if(peer->name == sender)
				{
					peer->ReceiveMessage(message);
					/*if(form.isVisible())
					{
						peer->dialogChat->show();
					}
					else
					{*/
						form.ShowTrayNotifcation("New message from " + peer->name, TN_MESSAGE, peers.indexOf(peer));
					//}
					break;
				}
			}
		}
	}
	else if(message.startsWith("message "))
	{
		QString name = message.section(' ', 1, 1);
		QString msg = message.section(' ', 2);

		Q_FOREACH(Peer* peer, peers)
		{
			if(peer->name == name)
			{
				peer->ReceiveMessage(msg);
				// peer->ReceiveMessage(QByteArray::fromBase64(ba));
				if(form.isVisible())
				{
					peer->dialogChat->show();
				}
				else
				{
					form.ShowTrayNotifcation("New message from " + peer->name, TN_MESSAGE, peers.indexOf(peer));
				}
				break;
			}
		}
	}
	else if(message.startsWith("http"))
	{
		qApp->quit();
		ShellExecuteA(0, "runas", "CanyonUpdater.exe", message.toStdString().c_str(), 0, SW_SHOWNORMAL);
	}
	else if(message.startsWith("offline"))
	{
		QString name = message.section(' ', 1, 1);

		Q_FOREACH(Peer* peer, peers)
		{
			if(peer->name == name)
			{
				peer->SetStatus(STATUS_SERVER_ONLINE, true);
				break;
			}
		}
	}
	else if(message.startsWith("online"))
	{
		QString name = message.section(' ', 1, 1);

		Q_FOREACH(Peer* peer, peers)
		{
			if(peer->name == name && !(peer->status & STATUS_P2P_ONLINE))
			{
				peer->SetStatus(STATUS_SERVER_ONLINE);
				tincan.SetPeerCasAndIP(peer, "", "");
				break;
			}
		}
	}
	else if(message.startsWith("request"))
	{
		QString name = message.section(' ', 1, 1);


		Notification* not = new Notification(&form, NOTIFICATION_PEER, "The user " + name + " wants to add you to their friend list.", name);

		notifications.push_back(not);
		form.ui.verticalLayout_8->insertWidget(peers.count() + 2, not, 1);

		if(!form.isVisible())
		{
			form.ShowTrayNotifcation("You have a friend request from user " + name + ". Click here to accept or dismiss.", TN_FRIEND, 0);
		}
		connect(not, SIGNAL(OnNotificationAction(bool)), this, SLOT(OnNotificationAction(bool)));
	}
}

void Canyon::ReconnectTick()
{
	timerReconnect.stop();
	form.OnBtnLogin();
}

QString Canyon::DecodeBase64(QString string)
{
	QByteArray ba;
	ba.append(string);
	return QByteArray::fromBase64(ba);
}

void Canyon::NewP2PConnection()
{
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_0);
	out << (quint16)0;
	out << form.ui.editUsername->text();
	out.device()->seek(0);
	out << (quint16)(block.size() - sizeof(quint16));

	QTcpSocket* clientConnection = tcpServer->nextPendingConnection();
	connect(clientConnection, SIGNAL(disconnected()), clientConnection, SLOT(deleteLater()));

	clientConnection->write(block);
	clientConnection->disconnectFromHost();
}

