#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

#include <QDialog>
#include "ui_configuredialog.h"
#include "../../../inc/bdaqctrl.h"

using namespace Automation::BDaq;

#define BUF_SIZE_MAX   2048
struct ConfigureParameter 
{
	QString deviceName;
    uint32  slnIdx;
    QString slnName;
};

class ConfigureDialog : public QDialog
{
	Q_OBJECT

public:
	ConfigureDialog(QWidget *parent = 0);
	~ConfigureDialog();
	void Initailization();
	void CheckError(ErrorCode errorCode);
	ConfigureParameter GetConfigureParameter(){return configure;}

private:
	Ui::ConfigureDialog ui;
	ConfigureParameter configure;
    CalibrationCtrl*   m_caliCtrl;

private slots:
    void DeviceChanged();
	void ButtonOKClicked();
	void ButtonCancelClicked();	
};

#endif // CONFIGUREDIALOG_H
