#ifndef ChatEdit_h__
#define ChatEdit_h__

#include <QtGui/QTextEdit>
#include <QWidget>
#include <QKeyEvent>
#include <QLabel>

class ChatEdit : public QTextEdit
{
	Q_OBJECT
public:
	ChatEdit(QWidget* parent) : QTextEdit(parent) { };

	void ChatEdit::keyPressEvent(QKeyEvent *event)
	{
		if (event->key() == Qt::Key_Return)
		{
			emit onTextReturn(); // or rather emit submitted() or something along this way
			event->accept();
		}
		else
		{
			QTextEdit::keyPressEvent(event);
		}
	}

signals:
	void onTextReturn(); 
};

class ClickableLabel : public QLabel
{
	Q_OBJECT
public:
	ClickableLabel(QWidget *parent = 0) : QLabel(parent) {}

signals:
	void clicked();

protected:
	void mousePressEvent(QMouseEvent *event) { emit clicked(); }
};


#endif // ChatEdit_h__
