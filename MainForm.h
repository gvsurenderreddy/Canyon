#ifndef MAINFORM_H
#define MAINFORM_H

#include <QWidget>
#include <QMovie>
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QSettings>
#include "ui_mainform.h"

class Canyon;

enum TRAY_NOTIFICATION
{
	TN_NONE,
	TN_MESSAGE,
	TN_FRIEND,
	TN_FILE,
	TN_BACKGROUND
};

class MainForm : public QMainWindow
{
	Q_OBJECT

public:
	MainForm(QWidget *parent = 0, Qt::WFlags flags = 0);

	void closeEvent(QCloseEvent* event);
	void ShowTrayNotifcation(QString message, TRAY_NOTIFICATION tn, int data);

	friend class Canyon;

public slots:
	void OnBtnLogin();
	void OnBtnRegister();
	void OnBtnCancelLoading();
	void OnLogout();
	void OnExit();
	void OnAddPeer();
	void OnOptions();
	void OnEditProfile();
	void OnHelp();
	void OnAbout();
	void OnRestore();
	void OnIconActivated(QSystemTrayIcon::ActivationReason reason);
	void OnTrayMessageClick();

private:
	QMovie* movie;
	QSystemTrayIcon* trayIcon;
	TRAY_NOTIFICATION notMotive;
	int notData;
	Ui::FormMain ui;
	//SimpleCrypt crypto;
	Canyon* canyon; // Ugly but simpler

public:
	static QSettings settings;
};

#endif // MAINFORM_H
