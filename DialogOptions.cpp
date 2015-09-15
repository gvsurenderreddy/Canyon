#include "DialogOptions.h"
#include <QDesktopServices>
#include <QMessageBox>
#include <QSettings.h>


DialogOptions::DialogOptions(QWidget *parent /*= 0*/, Qt::WFlags flags /*= 0*/)
	: QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint)
{
	ui.setupUi(this);
}

void DialogOptions::OnDefaults()
{
	ui.checkStartWindows->setChecked(true);
	ui.checkDtls->setChecked(true);
	ui.checkSaveLogs->setChecked(true);
	ui.checkRunBackground->setChecked(true);
	ui.checkOnDemand->setChecked(true);
	ui.checkAutoAccept->setChecked(false);

	ui.editIpv4->setText("172.31.0.100");
	ui.editLogs->setText(QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "\\Canyon\\Chat Logs");
	ui.editDownloadLocation->setText(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));
	ui.editPort->setText("62608");
	ui.editTimeout->setText("1500");
	ui.lineEdit->setText("fd50:0dbc:41f2:4a3c");
}