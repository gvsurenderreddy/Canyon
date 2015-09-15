#include "Peer.h"
#include <QMenu>
#include <QMessageBox>
#include <QApplication>
#include <QDialog>
#include <QWidget>
#include <QDateTime>
#include <QProcess>
#include <QClipboard>
#include <QFileDialog>
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>

Peer::Peer(QWidget *parent /*= 0*/, QString loc)
	: QWidget(parent),
	location(loc),
	dialogChat(0),
	status(0),
	tcpSocket(0)
{
	uiItem.setupUi(this);
	this->setContextMenuPolicy(Qt::CustomContextMenu);

	LoadChatHistory();

	dialogChat = new DialogChat(/*this->parentWidget()*/0);

	connect(dialogChat->ui.pushButton, SIGNAL(clicked()), this, SLOT(OnSendFile()));
	connect(dialogChat->ui.textEdit, SIGNAL(onTextReturn()), this, SLOT(OnPressEnter()));
	dialogChat->ui.label->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(dialogChat->ui.label, SIGNAL(customContextMenuRequested(const QPoint&)),
		this, SLOT(showContextMenu(const QPoint&)));

	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnContextMenu(const QPoint&)));
}


void Peer::SetStatus(int stat, bool neg)
{
	if(neg)
	{
		status &= ~stat;
	}
	else
	{
		status |= stat;
	}

	if(status & STATUS_P2P_ONLINE)
	{
		uiItem.labelStatus->setText(ip);
		uiItem.imageStatus->setStyleSheet("background: bottom url(:/MainForm/Resources/dot-green.png) no-repeat;");
		uiItem.labelUser->setStyleSheet("font: 14pt \"Segoe UI Light\"; color: rgb(200, 200, 200);");
		uiItem.labelStatus->setStyleSheet("font: 7pt \"Segoe UI\"; color: rgb(140, 140, 140);");
	}
	else if(status & STATUS_SERVER_ONLINE)
	{
		uiItem.labelStatus->setText("Online, establishing p2p link");
		uiItem.imageStatus->setStyleSheet("background: bottom url(:/MainForm/Resources/dot-yellow.png) no-repeat;");
		uiItem.labelUser->setStyleSheet("font: 14pt \"Segoe UI Light\"; color: rgb(200, 200, 200);");
		uiItem.labelStatus->setStyleSheet("font: 7pt \"Segoe UI\"; color: rgb(140, 140, 140);");
	}
	else if(status == 0)
	{
		uiItem.labelStatus->setText("Offline");
		uiItem.labelUser->setStyleSheet("font: 14pt \"Segoe UI Light\"; color: rgb(100, 100, 100);");
		uiItem.labelStatus->setStyleSheet("font: 7pt \"Segoe UI\"; color: rgb(99, 99, 99);");
		uiItem.imageStatus->setStyleSheet("background: bottom url(:/MainForm/Resources/dot-grey.png) no-repeat;");
	}
}

void Peer::SetName(QString _name)
{
	uiItem.labelUser->setText(_name);
	name = _name;
	dialogChat->setWindowTitle(name);
}

void Peer::OnContextMenu(const QPoint& pos)
{
	QPoint globalPos = this->mapToGlobal(pos); // Map the global position to the user list


	QMenu myMenu(this->parentWidget());
	QAction* actionStartChat = myMenu.addAction("Start chat");
	myMenu.addSeparator();
	QAction* actionCopyAddress = myMenu.addAction("Copy IPv4 address");
	QAction* actionSendFiles = myMenu.addAction("Check connection...");
	// actionSendFiles->setEnabled(false);
	QAction* actionPing = myMenu.addAction("Ping...");
	QAction* actionRemove = myMenu.addAction("Delete peer");
	// ...

	QAction* selectedItem = myMenu.exec(globalPos);
	if (selectedItem == actionCopyAddress)
	{
		// something was chosen, do stuff
		QApplication::clipboard()->setText(ip);
	}
	else if (selectedItem == actionStartChat)
	{
		dialogChat->show();
	}
	else if (selectedItem == actionSendFiles)
	{
		// nothing was chosen
		/*emit FileSend(this, QFileDialog::getOpenFileNames(
		this->parentWidget(),
		"Select one or more files to send",
		"/home",
		"Any files (*.*)"));*/
	
		if(!tcpSocket)
			tcpSocket = new QTcpSocket(this);
		connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(OnP2PReceive()));
		connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(OnP2PError(QAbstractSocket::SocketError)));
		tcpSocket->connectToHost(ip, 4644);
	}
	else if (selectedItem == actionPing)
	{
		QProcess process;
		process.startDetached("cmd.exe /K ping " + ip);
	}
	else if (selectedItem == actionRemove)
	{
		if(QMessageBox::question(this->parentWidget(), "Delete peer", "Are you sure you want to delete " 
			+ this->name + " from your friend list?", QMessageBox::Yes, 
			QMessageBox::No) == QMessageBox::Yes)
		{
			emit Remove(this);
		}
	}
}

void Peer::OnPressEnter()
{
	emit MessageSend(this, QDateTime::currentDateTime().toString("[MM/dd hh:mm] ") + dialogChat->ui.textEdit->toPlainText());

	QString message = "<span style=\"color: rgb(90, 90, 90);\">" +
		QDateTime::currentDateTime().toString("[MM/dd hh:mm] ") + 
		dialogChat->ui.textEdit->toPlainText() + "</span>";

	chatLines.append(message);
	dialogChat->ui.label->append(message);

	dialogChat->ui.label->ensureCursorVisible();

	dialogChat->ui.textEdit->clear();
}

void Peer::OnSendFile()
{
	emit FileSend(this, QFileDialog::getOpenFileNames(
		dialogChat,
		"Select one or more files to send",
		"/home",
		"Any files (*.*)"));
}

void Peer::ReceiveMessage(QString message)
{
	// message = QDateTime::currentDateTime().toString("[MM/dd hh:mm] ") + message;
	chatLines.append(message);
	dialogChat->ui.label->append(message);

	dialogChat->ui.label->ensureCursorVisible();
}

void Peer::LoadChatHistory()
{
	QFile file(location);
	file.open(QIODevice::ReadOnly | QIODevice::Text);

	QTextStream stream(&file); 
	while(!stream.atEnd())
	{
		chatLines.append(stream.readLine());
	} 
	file.close();
}

void Peer::SaveChatHistory()
{
	QFile file(location);
	file.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream out(&file);

	Q_FOREACH(QString line, chatLines)
	{
		out << line << "\n";
	}

	// optional, as QFile destructor will already do it:
	file.close();
}

void Peer::showContextMenu(const QPoint& pt)
{
	QMenu* menu = dialogChat->ui.label->createStandardContextMenu();
	// menu->addSeparator();
	QAction* actionLoadChatHistory = menu->addAction("Load chat history");
	QAction* actionClear = menu->addAction("Clear conversation");
	QAction* selectedItem = menu->exec(dialogChat->ui.label->mapToGlobal(pt));

	if (selectedItem == actionLoadChatHistory)
	{
		dialogChat->ui.label->clear();
		Q_FOREACH(QString message, chatLines)
		{
			dialogChat->ui.label->append(message);

			dialogChat->ui.label->ensureCursorVisible();
		}
	}
	else if (selectedItem == actionClear)
	{
		dialogChat->ui.label->clear();
	}
	delete menu;
}

Peer::~Peer()
{
	SaveChatHistory();
	delete dialogChat;
}

void Peer::OnP2PReceive()
{
	QDataStream in(tcpSocket);
	in.setVersion(QDataStream::Qt_4_0);

	quint16 blockSize = 0;

	if (tcpSocket->bytesAvailable() < (int)sizeof(quint16))
		return;

	in >> blockSize;

	if (tcpSocket->bytesAvailable() < blockSize)
		return;

	QString nextFortune;
	in >> nextFortune;

	if(nextFortune == name)
	{
		QMessageBox::information(this->parentWidget(), "Test connection", "The connection with " + name + " is working.");
	}
}

void Peer::OnP2PError(QAbstractSocket::SocketError error)
{
	if(error != 1)
		QMessageBox::warning(this->parentWidget(), "Test connection", "The connection with " + name + " failed with code " + QString::number(error) + ".");
}

DialogChat::DialogChat(QWidget *parent /*= 0*/, Qt::WFlags flags /*= 0*/)
	: QDialog(parent, flags)
{
	ui.setupUi(this);
	ui.progressBar->setVisible(false);

}
