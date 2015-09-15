#include "Notification.h"



Notification::Notification(QWidget* parent, NOTIFICATION_TYPE type, QString message, QString data)
	: QWidget(parent), m_data(data), m_type(type)
{
	ui.setupUi(this);
	ui.label_2->setText(message);

	/*if(type > 0)
	ui->setStyleSheet("image: url(:/MainForm/Resources/notification_file.png);");*/

	if(type < NOTIFICATION_PROGRESS)
		ui.progressBar->setVisible(false);
	else
		ui.NCancel->setVisible(false);

	connect(ui.NCancel, SIGNAL(clicked()), this, SLOT(OnBtnCancel()));
	connect(ui.NAccept, SIGNAL(clicked()), this, SLOT(OnBtnAccept()));
}

void Notification::SetProgress(int progress)
{
	ui.progressBar->setValue(progress);
}

void Notification::OnBtnAccept()
{
	emit OnNotificationAction(true);
}

void Notification::OnBtnCancel()
{
	emit OnNotificationAction(false);
}
