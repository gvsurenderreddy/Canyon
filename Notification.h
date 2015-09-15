#ifndef Notification_h__
#define Notification_h__

#include <QtGui/QWidget>
#include <QtGui/QItemDelegate>

#include "ui_itemnotification.h"

enum NOTIFICATION_TYPE
{
	NOTIFICATION_PEER,
	NOTIFICATION_DOWNLOAD,
	NOTIFICATION_PROGRESS
};

class Notification : public QWidget
{
	Q_OBJECT
public:
	Notification(QWidget* parent, NOTIFICATION_TYPE type, QString message, QString data);
	void SetProgress(int progress);

signals:
	void OnNotificationAction(bool);

private slots:
	void OnBtnAccept();
	void OnBtnCancel();

public:
	NOTIFICATION_TYPE m_type;
	QString m_data;
	QString m_data2;

private:
	Ui::ItemNotification ui;
};

#endif // Notification_h__
