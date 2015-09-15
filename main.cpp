#include "Canyon.h"
#include <QtGui/QApplication>
#include <QThread>
#include <windows.h>
#include "MainForm.h"

typedef int (*GW)();

class MyThread : public QThread
{
	//Q_OBJECT
public:
	GW GetWelcomeMessage;

protected:
	void run();
};

void MyThread::run()
{

	GetWelcomeMessage();
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QStringList args = a.arguments();

	if(args.count() >= 2 && args[1] == "/setup")
	{
		QSettings settings("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}", QSettings::NativeFormat);

		QString id;

		Q_FOREACH(QString key, settings.childGroups())
		{
			QSettings s1("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\" + key, QSettings::NativeFormat);

			if(s1.value("DriverDesc").toString() == "TAP-Windows Adapter V9")
			{
				id = s1.value("NetCfgInstanceId").toString();
				break;
			}

		}

		QSettings s2("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\" + id + "\\Connection", QSettings::NativeFormat);


		QString name = s2.value("Name").toString();

		if(id == "" || name == "")
		{
			QMessageBox::critical(0, "Fatal error", "The TAP Network adapter is not installed. Please reinstall this application.");
			return -1;
		}

		QProcess process;
		process.start("netsh interface set interface name=\"" + name + "\" newname=\"Canyon\"");
		process.waitForFinished();
		process.start("netsh interface ip set address Canyon static " + MainForm::settings.value("ipv4Base", "172.31.0.100").toString() + " 255.255.255.0");
		process.waitForFinished();
		process.start("netsh interface ipv4 set subinterface Canyon mtu=1280 store=persistent");
		process.waitForFinished();


		/*QSettings s3("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\" + id + "\\Connection", QSettings::NativeFormat);

		s3.setValue("Name", "Canyon");*/

		return -1;
	}


	HMODULE hModule = LoadLibrary(TEXT("Canyon.dll"));
	GW GetWelcomeMessage = (GW) GetProcAddress(hModule, "main");

	if(GetWelcomeMessage == 0)
	{
		QMessageBox::critical(0, "Fatal error", "Couldn't load service DLL. Please reinstall the application.");
		return -1;
	}

	MyThread thread;
	thread.GetWelcomeMessage = GetWelcomeMessage;
	thread.start();

	QString cssFile;

	if(QFile::exists("skin.css"))
		cssFile = "skin.css";
	else
		cssFile = ":/MainForm/Resources/dark.css";

	QFile file(cssFile);
	file.open(QIODevice::ReadOnly | QIODevice::Text);

	QString css;

	QTextStream in(&file);
	while (!in.atEnd()) 
	{
		css += in.readLine() + "\n";
	}

	a.setStyleSheet(css);

	Canyon canyon;

	return a.exec();

}
