#include "configuredialog.h"
#include <QMessageBox>
#include <QtDebug>
#include <QProcess>
#include <QFileDialog>

#define BUF_SIZE   2048

ConfigureDialog::ConfigureDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	//Set the minimum and close button of the main frame.
	this->setWindowFlags(Qt::WindowFlags(Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint));

    connect(ui.cmbDevice, SIGNAL(currentIndexChanged(int)), this, SLOT(DeviceChanged()));
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(ButtonOKClicked()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(ButtonCancelClicked()));	
	
	Initailization();
}

ConfigureDialog::~ConfigureDialog()
{
    m_caliCtrl->Dispose();
}

void ConfigureDialog::Initailization()
{
    m_caliCtrl = CalibrationCtrl::Create();
    Array<DeviceTreeNode> *supportedDevices = m_caliCtrl->getSupportedDevices();

	if (supportedDevices->getCount() == 0)
	{
		QMessageBox::information(this, tr("Warning Information"), 
			tr("No device to support the currently demonstrated function!"));
		QCoreApplication::quit();
	} else {
		for (int i = 0; i < supportedDevices->getCount(); i++) {
			DeviceTreeNode const &node = supportedDevices->getItem(i);
			qDebug("%d, %ls\n", node.DeviceNumber, node.Description);
			ui.cmbDevice->addItem(QString::fromWCharArray(node.Description));
		}
		ui.cmbDevice->setCurrentIndex(0);	
	}	
	supportedDevices->Dispose();   
}

void ConfigureDialog::CheckError(ErrorCode errorCode)
{
	if (errorCode >= 0xE0000000 && errorCode != Success)
	{
		QString message = tr("Sorry, there are some errors occurred, Error Code: 0x") +
			QString::number(errorCode, 16).right(8).toUpper();
		QMessageBox::information(this, "Warning Information", message);
	}
}

void ConfigureDialog::DeviceChanged()
{
   ui.cmbSolution->clear();

   std::wstring description = ui.cmbDevice->currentText().toStdWString();
   DeviceInformation selected(description.c_str());
   ErrorCode errorCode = m_caliCtrl->setSelectedDevice(selected);
   ui.btnOK->setEnabled(true);
   if (errorCode != Success){
       QString str;
       QString des = QString::fromStdWString(description);
       str.sprintf("Error:the error code is 0x%x\n\
						The %s is busy or not exit in computer now.\n\
                        Select other device please!", errorCode, des.toUtf8().data());
       QMessageBox::information(this, "Warning Information", str);
       ui.btnOK->setEnabled(false);
       return;
   }

   //solution info   
   wchar_t const * str_ptr = 0;
   Array<CaliSolution>* slnList = m_caliCtrl->getSolutions();
   qDebug() << "support soultion items count: " << slnList->getCount();

   ui.cmbSolution->blockSignals(true);
   for(int32 i = 0; i < slnList->getCount(); i++)
   {       
       str_ptr = slnList->getItem(i).getDescription();              
       qDebug("solution Index [%d], str_ptr[%ls]\n",i, str_ptr);
       ui.cmbSolution->addItem(QString::fromWCharArray(str_ptr));
   }
   ui.cmbSolution->setCurrentIndex(0); //default value;
   ui.cmbSolution->blockSignals(false);   
}

void ConfigureDialog::ButtonOKClicked()
{
	if (ui.cmbDevice->count() == 0){
		QCoreApplication::quit();
	}	

    configure.deviceName = ui.cmbDevice->currentText();
    configure.slnIdx = ui.cmbSolution->currentIndex();
    configure.slnName = ui.cmbSolution->currentText();

    //m_caliCtrl->Dispose();
	this->accept();
}

void ConfigureDialog::ButtonCancelClicked()
{
	this->reject();
}

