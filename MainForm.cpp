#include <QMessageBox>

#include "MainForm.h"
#include "Canyon.h"
#include "SlidingStackedWidget.h"
#include "DialogOptions.h"
#include "ui_dialogaddpeer.h"
//#include "AESc.h"

QSettings MainForm::settings(QSettings::NativeFormat, QSettings::UserScope, "Canyon", "Canyon");

MainForm::MainForm(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags),
	notMotive(TN_NONE),
	notData(0),
	canyon(0),
	//crypto(Q_UINT64_C(0x0d7ac79a0f0abd10)),
	trayIcon(new QSystemTrayIcon(this))
{
	// Setup UI
	ui.setupUi(this);
	QApplication::instance()->setAttribute(Qt::AA_DontShowIconsInMenus);

	ui.labelError->hide(); // Hide error label, front page
	ui.stackedWidget->setCurrentIndex(0);


	movie = new QMovie(":/MainForm/Resources/loading.gif");
	movie->setScaledSize(QSize(32, 32));
	ui.imageLoading->setMovie(movie);

	trayIcon = new QSystemTrayIcon(this);
	this->setWindowIcon(QIcon(":/MainForm/Resources/icon.png"));
	trayIcon->setIcon(QIcon(":/MainForm/Resources/icon.png"));
	trayIcon->show();
	trayIcon->setToolTip("Canyon");
	// TODO: Make tray icon configurable trayIcon->hide();
	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(OnIconActivated(QSystemTrayIcon::ActivationReason)));
	connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(OnTrayMessageClick()));

	QMenu* trayMenu = new QMenu(this); // Create tray menu

	QAction* restoreAction = new QAction("Restore", this);
	connect(restoreAction, SIGNAL(triggered()), this, SLOT(OnRestore()));
	trayMenu->addAction(restoreAction);
	trayMenu->addSeparator();

	QAction* optionsAction = new QAction("Options", this);
	connect(optionsAction, SIGNAL(triggered()), this, SLOT(OnOptions()));
	trayMenu->addAction(optionsAction);

	QAction* helpAction = new QAction("Help", this);
	connect(helpAction, SIGNAL(triggered()), this, SLOT(OnHelp()));
	trayMenu->addAction(helpAction);

	QAction* aboutAction = new QAction("About", this);
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(OnAbout()));
	trayMenu->addAction(aboutAction);
	trayMenu->addSeparator();

	QAction* quitAction = new QAction("Quit", this);
	connect(quitAction, SIGNAL(triggered()), this, SLOT(OnExit()));
	trayMenu->addAction(quitAction);

	trayIcon->setContextMenu(trayMenu);

	// Load settings

	ui.editUsername->setText(settings.value("username").toString());

	if(settings.value("password").toString() != "")
	{
//		QString decPass = crypto.decryptToString(settings.value("password").toString());
//		ui.editPassword->setText(decPass);
	}

	ui.editUsername->setText(settings.value("username").toString());
	ui.checkAutoLogin->setChecked(settings.value("autoLogin", true).toBool());


	
}

void MainForm::OnBtnLogin()
{
	/*QString encrypted = AESc::Encrypt("test");
	QMessageBox::information(this, "enc", encrypted);
	QString decrypted = AESc::Decrypt(encrypted);
	QMessageBox::information(this, "dec", decrypted);*/


	if(ui.editUsername->text().isEmpty() && ui.editPassword->text().isEmpty())
	{
		ui.labelError->setText("Please enter your username and password.");
		ui.labelError->show();
	}
	else if(ui.editUsername->text().isEmpty())
	{
		ui.labelError->setText("Please enter your username.");
		ui.labelError->show();
	}
	else if(ui.editPassword->text().isEmpty())
	{
		ui.labelError->setText("Please enter your password.");
		ui.labelError->show();
	}
	else
	{
		ui.labelError->hide();

		ui.stackedWidget->slideInIdx(1, SlidingStackedWidget::TOP2BOTTOM);

		movie->start();

		// Save settings
		settings.setValue("username", ui.editUsername->text());

		if(ui.checkAutoLogin->isChecked())
		{
//			QString encPass = crypto.encryptToString(ui.editPassword->text());
//			settings.setValue("password", encPass);
			settings.setValue("autoLogin", true);
		}
		else
		{
			settings.remove("password");
			settings.setValue("autoLogin", false);
		}


		// Do login
		canyon->Login();
	}
}

void MainForm::OnAddPeer()
{
	QDialog dialogAddPeer(this, Qt::Dialog | Qt::WindowCloseButtonHint);
	Ui::FormAddPeer u;
	u.setupUi(&dialogAddPeer);
	dialogAddPeer.setFixedSize(dialogAddPeer.width(), dialogAddPeer.height());
	if(dialogAddPeer.exec() == QDialog::Accepted)
	{
		QString name = u.lineEdit->text();

		if(name.length() > 3)
		{
			Q_FOREACH(Peer* peer, canyon->peers)
			{
				if(peer->name == name)
				{
					return;
				}
			}
			canyon->AddPeer(name);
			canyon->server.AddPeer(name);
			QMessageBox::information(this, "Add Peer", "A request has been sent to the user.");
		}
		else
		{
			QMessageBox::warning(this, "Add Peer", "Username not valid.");
		}
	}
}


void MainForm::OnTrayMessageClick()
{
	this->show();
	if(notMotive == TN_BACKGROUND)
	{
		OnRestore();
	}
	else if(notMotive == TN_MESSAGE)
	{
		Peer* peer = canyon->peers.at(notData);
		peer->dialogChat->show();
	}
	else if(notMotive == TN_FRIEND || notMotive == TN_FILE)
	{
		this->show();
		this->activateWindow();
	}

	notMotive = TN_NONE;
}

void MainForm::OnBtnCancelLoading()
{
	canyon->Reset();
	ui.stackedWidget->slideInIdx(0, SlidingStackedWidget::BOTTOM2TOP);
}

void MainForm::OnLogout()
{
	canyon->Logout();
	ui.stackedWidget->slideInIdx(0, SlidingStackedWidget::LEFT2RIGHT);
}

void MainForm::OnBtnRegister()
{
	QDesktopServices::openUrl(QUrl("https://canyonvpn.com/register"));
}

void MainForm::OnExit()
{
	trayIcon->hide();
	QApplication::quit();
}

void MainForm::OnAbout()
{
	QMessageBox::about(this, "About Canyon", "Canyon VPN LAN software\nVersion " CANYON_VERSION ", " __DATE__ "\ncanyonvpn.com");
}

void MainForm::OnHelp()
{
	QDesktopServices::openUrl(QUrl("http://canyonvpn.com/help"));
}

void MainForm::OnEditProfile()
{
	QDesktopServices::openUrl(QUrl("http://canyonvpn.com/profile"));
}

void MainForm::OnOptions()
{
	DialogOptions dialogOptions(this);

	dialogOptions.ui.checkStartWindows->setChecked(settings.value("startWindows", true).toBool());
	dialogOptions.ui.checkDtls->setChecked(settings.value("enableDtls", true).toBool());
	dialogOptions.ui.checkSaveLogs->setChecked(settings.value("saveLogs", true).toBool());
	dialogOptions.ui.checkRunBackground->setChecked(settings.value("minimizeToTray", true).toBool());
	dialogOptions.ui.checkOnDemand->setChecked(settings.value("onDemandConnection", true).toBool());
	dialogOptions.ui.checkAutoAccept->setChecked(settings.value("autoAcceptTransfers", false).toBool());

	dialogOptions.ui.editIpv4->setText(settings.value("ipv4Base", "172.31.0.100").toString());
	dialogOptions.ui.editLogs->setText(settings.value("chatLogsDirectory", 
		QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "\\Canyon\\Chat Logs").toString());
	dialogOptions.ui.editDownloadLocation->setText(settings.value("downloadsDirectory", 
		QDesktopServices::storageLocation(QDesktopServices::DesktopLocation)).toString());
	dialogOptions.ui.editPort->setText(settings.value("canyonPort", "62608").toString());
	dialogOptions.ui.editTimeout->setText(settings.value("timeoutMs", "1500").toString());
	dialogOptions.ui.lineEdit->setText(settings.value("ipv6Base", "fd50:0dbc:41f2:4a3c").toString());

	if(dialogOptions.exec() == QDialog::Accepted)
	{
		settings.setValue("startWindows", dialogOptions.ui.checkStartWindows->isChecked());
		settings.setValue("enableDtls", dialogOptions.ui.checkDtls->isChecked());
		settings.setValue("saveLogs", dialogOptions.ui.checkSaveLogs->isChecked());
		settings.setValue("minimizeToTray", dialogOptions.ui.checkRunBackground->isChecked());
		settings.setValue("onDemandConnection", dialogOptions.ui.checkOnDemand->isChecked());
		settings.setValue("autoAcceptTransfers", dialogOptions.ui.checkAutoAccept->isChecked());

		settings.setValue("ipv4Base", dialogOptions.ui.editIpv4->text());
		settings.setValue("chatLogsDirectory", dialogOptions.ui.editLogs->text());
		settings.setValue("downloadsDirectory", dialogOptions.ui.editDownloadLocation->text());
		settings.setValue("canyonPort", dialogOptions.ui.editPort->text());
		settings.setValue("timeoutMs", dialogOptions.ui.editTimeout->text());
		settings.setValue("ipv6Base", dialogOptions.ui.lineEdit->text());

		if (QMessageBox::question(&dialogOptions, "Canyon", "The application needs to be restarted"
			" for changes to take effect. Would you like to restart now?", QMessageBox::Yes|QMessageBox::No)
			== QMessageBox::Yes) 
		{
			QProcess::startDetached(QApplication::applicationFilePath());
			trayIcon->hide();
			QApplication::quit();
		}


	}
}

void MainForm::closeEvent(QCloseEvent* event)
{
	if(settings.value("minimizeToTray", true).toBool())
	{
		// trayIcon->show();
		if(!settings.value("minimizeTrayMessageShown", false).toBool())
		{
			ShowTrayNotifcation("Canyon is still running in the background."
				" You can reopen it by double clicking on the system tray icon or "
				"you can go to Options to disable this behaviour.", TN_BACKGROUND, 0);

			settings.setValue("minimizeTrayMessageShown", true);
		}
		hide();
		event->ignore();
	}
	else
	{

		trayIcon->hide();

		QMainWindow::closeEvent(event);
		QApplication::quit();
		//
	}
}

void MainForm::OnRestore()
{
	this->show();
	// trayIcon->hide();
}

void MainForm::OnIconActivated(QSystemTrayIcon::ActivationReason reason)
{
	if(reason == QSystemTrayIcon::DoubleClick)
	{
		OnTrayMessageClick();
	}
	else
	{
		this->setWindowState( (windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
		
		this->activateWindow();
	}
}

void MainForm::ShowTrayNotifcation(QString message, TRAY_NOTIFICATION tn, int data)
{
	notMotive = tn;
	notData = data;
	trayIcon->showMessage("Canyon", message, QSystemTrayIcon::Information, 5000);
}
