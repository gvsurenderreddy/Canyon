#ifndef DialogOptions_h__
#define DialogOptions_h__

#include <QtGui/QDialog>

#include "ui_optionsform.h"

class DialogOptions : public QDialog
{
	Q_OBJECT

public:
	DialogOptions(QWidget *parent = 0, Qt::WFlags flags = 0);

	Ui::FormOptions ui;

private slots:
	void OnDefaults();
};

#endif // DialogOptions_h__
