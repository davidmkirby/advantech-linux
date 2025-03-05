#include <QPalette>
#include <QMessageBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDebug>
#include "calibrationitem.h"

#define RELATED_CNT_MAX      8
#define TABLE_COLUMN_COUNT   3

CalibrationItem::CalibrationItem(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags), m_thread(this)
{
    qRegisterMetaType<CaliState>("CaliState");

	ui.setupUi(this);
	//Set the minimum and close button of the main frame.
	this->setWindowFlags(Qt::WindowFlags(Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint));

    m_caliCtrl = CalibrationCtrl::Create();

    connect(ui.btn_start,        SIGNAL(clicked()),                 this, SLOT(ButtonStartClicked()));
    connect(ui.btn_stop,         SIGNAL(clicked()),                 this, SLOT(ButtonStopClicked()));
    connect(ui.btn_config,       SIGNAL(clicked()),                 this, SLOT(ButtonConfigureClicked()));
    connect(ui.btn_save,         SIGNAL(clicked()),                 this, SLOT(ButtonSaveClicked()));
    connect(ui.spb_adj_code,     SIGNAL(valueChanged(int)),         this, SLOT(SpinBoxClicked()));    
    connect(ui.cmb_sect_desc,    SIGNAL(currentIndexChanged(int)),  this, SLOT(onCmbSect()));
    connect(ui.cmb_subj_desc,    SIGNAL(currentIndexChanged(int)),  this, SLOT(onCmbSubj()));

    connect(this,                SIGNAL(update_state_signal(int, CaliState)),  this, SLOT(on_update_state(int, CaliState)));
    connect(this,                SIGNAL(update_val_signal(int)),               this, SLOT(on_update_value(int)));    
}

CalibrationItem::~CalibrationItem()
{
    m_strPtr = 0;
    m_thread.terminate();
    if(m_caliCtrl){
       m_caliCtrl->Dispose();
    }
}

void CalibrationItem::Initialize() {
	//set the title of the form.
    this->setWindowTitle(tr("Calibration Item - Run(") + configure.deviceName + tr(")"));
    qDebug() << "Device Name[" << configure.deviceName << "], Solution Name[" << configure.slnName << "], Solution Index[" << configure.slnIdx << "].";
	
    ui.btn_start->setEnabled(true);
    ui.btn_stop->setEnabled(false);
    ui.btn_save->setEnabled(false);
    ui.btn_config->setEnabled(true);
    ui.spb_adj_code->setEnabled(false);
    ui.lineEdit_read_value->setEnabled(false);

    m_sln_idx  = 0;
    m_sect_idx = 0;
    m_subj_idx = 0;

    m_sln_list  = 0;
    m_sect_list = 0;
    m_subj_list = 0;

    m_strPtr = 0;
    m_cali_stop = true;

    memset(m_subjCaliDone, 0, sizeof(m_subjCaliDone));

    ConfigureDevice();
}

void CalibrationItem::ConfigureDevice() {
    std::wstring description = configure.deviceName.toStdWString();
    DeviceInformation selected(description.c_str());

    ErrorCode errorCode = m_caliCtrl->setSelectedDevice(selected);
    CheckError(errorCode);

    //Get solution description based on index;    
    m_sln_idx = configure.slnIdx;
    m_sln_list = m_caliCtrl->getSolutions();    
    m_strPtr = m_sln_list->getItem(m_sln_idx).getDescription();

    //check info again;
    int is_matching = !(QString::compare(configure.slnName, QString::fromWCharArray(m_strPtr)));
    if(!is_matching){
        qDebug() << "The description based on Solution index DO NOT match to driver.";
    }

    //Solution Info
    m_strPtr = m_sln_list->getItem(m_sln_idx).getDescription();    
    ui.lbl_sln_desc->setText(QString::fromWCharArray(m_strPtr));

    m_strPtr = m_sln_list->getItem(m_sln_idx).getInstruction();    
    ui.textEdit_sln_instrction->setText(QString::fromWCharArray(m_strPtr));

    // Section Info
    m_sect_list = m_sln_list->getItem(m_sln_idx).getSections();

    //add section description
    ui.cmb_sect_desc->blockSignals(true);
    ui.cmb_sect_desc->clear();
    for(int32 i = 0; i < m_sect_list->getCount(); i++)
    {
        m_strPtr = m_sect_list->getItem(i).getDescription();                        
        ui.cmb_sect_desc->addItem(QString::fromWCharArray(m_strPtr));
    }

    ui.cmb_sect_desc->setCurrentIndex(0); //default value;
    ui.cmb_sect_desc->blockSignals(false);    
    m_sect_idx =  ui.cmb_sect_desc->currentIndex();

    //add section instrction
    m_strPtr = m_sect_list->getItem(m_sect_idx).getInstruction();    
    ui.textEdit_sect_instrc->setText(QString::fromWCharArray(m_strPtr));

    SectionTypeCheck();

    // Subject Info
    m_subj_list = m_sect_list->getItem(m_sect_idx).getSubjects();

    if(m_matching_ai_manual || m_matching_ao_manual){
        //add subject description
        ui.cmb_subj_desc->blockSignals(true);
        ui.cmb_subj_desc->clear();
        for(int32 i = 0; i < m_subj_list->getCount(); i++)
        {
            m_strPtr = m_subj_list->getItem(i).getDescription();            
            ui.cmb_subj_desc->addItem(QString::fromWCharArray(m_strPtr));
        }
        ui.cmb_subj_desc->setCurrentIndex(0); //default value;
        ui.cmb_subj_desc->blockSignals(false);
        m_subj_idx = ui.cmb_subj_desc->currentIndex();

        //add subject instrction
        m_strPtr = m_subj_list->getItem(m_subj_idx).getInstruction();        
        ui.textEdit_subj_instrc->setText(QString::fromWCharArray(m_strPtr));

        // add subject target
        m_strPtr = m_subj_list->getItem(m_subj_idx).getTargetRange();        
        ui.lbl_subj_target->setText(QString::fromWCharArray(m_strPtr));

        double value = 0;
        int32  codeCount = 0;
        int32  adjustCode[RELATED_CNT_MAX] = {0};

        CaliState caliState = m_subj_list->getItem(m_subj_idx).AdjCodeGetCurrentState(&value, &codeCount, adjustCode);
        qDebug() << "---ConfigureDevice---Cali State[" << caliState << "], Value:" << value << ", Codde Count:" << codeCount << ", Adjust Code[" << codeCount - 1 << "] = " << adjustCode[codeCount - 1];

        ui.lineEdit_read_value->setText(QString::number(value));

        // add subject code range
        int32 lowerValue[RELATED_CNT_MAX] = {0};
        int32 upperValue[RELATED_CNT_MAX] = {0};
        m_subj_list->getItem(m_subj_idx).AdjCodeGetRange(lowerValue, upperValue);

        int32 idx = codeCount - 1;
        ui.spb_adj_code->blockSignals(true);
        ui.spb_adj_code->setRange(lowerValue[idx], upperValue[idx]);
        ui.spb_adj_code->setValue(adjustCode[idx]);
        ui.spb_adj_code->blockSignals(false);
    }

    if(m_matching_ai_auto || m_matching_ao_auto){
        InitTableList();
    }

    SettingWidgetsState();
}

void CalibrationItem::onCmbSect()
{    
    m_sect_idx = ui.cmb_sect_desc->currentIndex();

    //add section instrction
    m_strPtr = m_sect_list->getItem(m_sect_idx).getInstruction();    
    ui.textEdit_sect_instrc->setText(QString::fromWCharArray(m_strPtr));

    SectionTypeCheck();

    onCmbSubj();
}

void CalibrationItem::onCmbSubj()
{        
    m_subj_list = m_sect_list->getItem(m_sect_idx).getSubjects();
    if(sender() != ui.cmb_subj_desc){
        SubjectChangeForWidgetsClear();

        //add subject description
        ui.cmb_subj_desc->blockSignals(true);
        for(int32 i = 0; i < m_subj_list->getCount(); i++)
        {
            m_strPtr = m_subj_list->getItem(i).getDescription();                        
            ui.cmb_subj_desc->addItem(QString::fromWCharArray(m_strPtr));
        }
        ui.cmb_subj_desc->blockSignals(false);
    }

    if(m_matching_ai_manual || m_matching_ao_manual){
        m_subj_idx = ui.cmb_subj_desc->currentIndex();
        //add subject instrction
        m_strPtr = m_subj_list->getItem(m_subj_idx).getInstruction();        
        ui.textEdit_subj_instrc->setText(QString::fromWCharArray(m_strPtr));

        // add subject target
        m_strPtr = m_subj_list->getItem(m_subj_idx).getTargetRange();        
        ui.lbl_subj_target->setText(QString::fromWCharArray(m_strPtr));

        double value = 0;
        int32  codeCount = 0;
        int32  adjustCode[RELATED_CNT_MAX] = {0};
        CaliState caliState = m_subj_list->getItem(m_subj_idx).AdjCodeGetCurrentState(&value, &codeCount, adjustCode);
        qDebug() << "---onCmbSubj---Cali State[" << caliState << "], Value:" << value << ", Codde Count:" << codeCount << ", Adjust Code[" << codeCount - 1 << "] = " << adjustCode[codeCount - 1];

        ui.lineEdit_read_value->setText(QString::number(value));

        // add subject code range
        int32 lowerValue[RELATED_CNT_MAX] = {0};
        int32 upperValue[RELATED_CNT_MAX] = {0};
        m_subj_list->getItem(m_subj_idx).AdjCodeGetRange(lowerValue, upperValue);

        int32 idx = codeCount - 1;
        ui.spb_adj_code->blockSignals(true);
        ui.spb_adj_code->setRange(lowerValue[idx], upperValue[idx]);
        ui.spb_adj_code->setValue(adjustCode[idx]);
        ui.spb_adj_code->blockSignals(false);
    }

    if(m_matching_ai_auto || m_matching_ao_auto){
        InitTableList();
    }

    SettingWidgetsState();
}

void CalibrationItem::InitTableList()
{
    ui.table_list_result->clear();

    int32 tableRowCount = m_subj_list->getCount();

    ui.table_list_result->setColumnCount(TABLE_COLUMN_COUNT);
    ui.table_list_result->setRowCount(tableRowCount);

    QStringList headerLables;
    headerLables << "Step" << "Description" << "Result";
    ui.table_list_result->setHorizontalHeaderLabels(headerLables);
    ui.table_list_result->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    //ui.table_list_result->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui.table_list_result->horizontalHeader()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    ui.table_list_result->horizontalHeader()->setStretchLastSection(true);

    ui.table_list_result->verticalHeader()->setVisible(false);
    ui.table_list_result->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui.table_list_result->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QString str("Step%1");
    for(int32 i = 0; i < tableRowCount; i++)
    {
        ui.table_list_result->setRowHeight(i, 18);

        ui.table_list_result->setItem(i, 0, new QTableWidgetItem(str.arg(i + 1)));

        m_strPtr = m_subj_list->getItem(i).getDescription();        
        ui.table_list_result->setItem(i, 1, new QTableWidgetItem(QString::fromWCharArray(m_strPtr)));

        ui.table_list_result->setItem(i, 2, new QTableWidgetItem("Idle"));
    }
}

void CalibrationItem::SectionChangeForWidgetsClear()
{
    ui.cmb_sect_desc->blockSignals(true);
    ui.cmb_sect_desc->clear();
    ui.cmb_sect_desc->blockSignals(false);

    ui.cmb_subj_desc->blockSignals(true);
    ui.cmb_subj_desc->clear();
    ui.cmb_subj_desc->blockSignals(false);
}

void CalibrationItem::SubjectChangeForWidgetsClear()
{
    ui.cmb_subj_desc->blockSignals(true);
    ui.cmb_subj_desc->clear();
    ui.cmb_subj_desc->blockSignals(false);

    ui.textEdit_subj_instrc->clear();
    ui.lbl_subj_target->clear();
    ui.lineEdit_read_value->clear();

    ui.spb_adj_code->blockSignals(true);
    ui.spb_adj_code->clear();
    ui.spb_adj_code->blockSignals(false);
}

void CalibrationItem::ButtonStartClicked()
{
    ErrorCode ret = Success;

    //update UI
    ui.btn_config->setEnabled(false);
    ui.btn_start->setEnabled(false);
    ui.btn_stop->setEnabled(true);

    ui.cmb_sect_desc->setEnabled(false);
    ui.cmb_subj_desc->setEnabled(false);

    if(m_matching_ai_auto || m_matching_ao_auto){
        int32 tableRowCount = m_subj_list->getCount();
        for(int32 i = 0; i < tableRowCount; i++)
        {
            ui.table_list_result->setItem(i, 2, new QTableWidgetItem("---NA---"));
        }
        m_thread.start();        

        ret = m_sect_list->getItem(m_sect_idx).BatchStart();        
    }

    if(m_matching_ai_manual || m_matching_ao_manual){        
        ui.spb_adj_code->setEnabled(true);

        //Manual AO don't need Timer
        if(m_matching_ai_manual){            
            m_thread.start();
        }

        ret = m_subj_list->getItem(m_subj_idx).ManualAdjStart();
    }
}

void CalibrationItem::ButtonStopClicked()
{
    ui.btn_config->setEnabled(true);
    ui.btn_start->setEnabled(true);
    ui.btn_stop->setEnabled(false);

    ui.cmb_sect_desc->setEnabled(true);
    ui.cmb_subj_desc->setEnabled(true);

    if(m_matching_ai_auto || m_matching_ao_auto){        
        m_sect_list->getItem(m_sect_idx).BatchTerminate();        
    }

    ErrorCode ret = Success;
    if(m_matching_ai_manual || m_matching_ao_manual){        
        ui.btn_save->setEnabled(true);
        ui.spb_adj_code->setEnabled(false);

        ret = m_subj_list->getItem(m_subj_idx).ManualAdjStop();

        //timer1->stop();
    }

    m_thread.wait(10);
    if(m_thread.isRunning()){
        m_thread.terminate();
    }
}

void CalibrationItem::ButtonConfigureClicked() {
	int dialogResult = configDialog->exec();
	if (dialogResult == QDialog::Accepted)
	{
		configure = configDialog->GetConfigureParameter();
		Initialize();
	}	
}

void CalibrationItem::ButtonSaveClicked()
{    
    m_sln_list->getItem(m_sln_idx).AdjCodesSave();
}

void CalibrationItem::SpinBoxClicked()
{    
    ui.btn_save->setEnabled(true);    
    int32 cali_data = ui.spb_adj_code->value();    
    ErrorCode ret = m_subj_list->getItem(m_subj_idx).ManualAdjSetCode(1, &cali_data);
    qDebug("SpinBoxClicked-->data = %d\n", cali_data);
}
void CalibrationItem::TimerTicked()
{    
    double  value = 0;
    int32   codeCount = 0;
    int32   adjustCode[RELATED_CNT_MAX] = {0};
    QString str_cali_state = "xxxxxx";

    int32   tableRowCount = m_subj_list->getCount();
    int32   lastOneIdx    = m_subj_list->getCount() - 1;

    CaliState caliState = CaliIdle;

    for(int32 i = 0; i < tableRowCount; i++)
    {
        caliState = m_subj_list->getItem(i).AdjCodeGetCurrentState(&value, &codeCount, adjustCode);
        qDebug() << "--TimerTicked--Row[" << i << "], Cali State[" << caliState << "], Value:" << value << ", Codde Count:" << codeCount << ", Adjust Code = " << adjustCode[0];

        if(caliState != CaliIdle)
        {
            switch((int)caliState)
            {
            case 0:
                str_cali_state = "Success";
                break;
            case 1:
                str_cali_state = "Failed";
                break;
            case 2:
                str_cali_state = "Running";
                break;
            default:
                str_cali_state = "--NA--";
                break;
            }

            ui.table_list_result->setItem(i, 2, new QTableWidgetItem(str_cali_state));
        }

        if((i == lastOneIdx) && (caliState == CaliSuccess || caliState == CaliFailed)){
            ui.btn_save->setEnabled(true);
            ButtonStopClicked();            
            //timer->stop();
            m_thread.terminate();
            qDebug() << "Auto Cali Finish~~";
        }
    }
}

void CalibrationItem::TimerTicked1()
{
    double  value = 0;
    int32   codeCount = 0;
    int32   adjustCode[RELATED_CNT_MAX] = {0};

    CaliState caliState = m_subj_list->getItem(m_subj_idx).AdjCodeGetCurrentState(&value, &codeCount, adjustCode);

    int32   idx = codeCount - 1;
    ui.lineEdit_read_value->setText(QString::number(value));
    qDebug() << "---TimerTicked-1---Cali State[" << caliState << "], Value:" << value << ", Codde Count:" << codeCount << ", Adjust Code[" << idx << "] = " << adjustCode[idx];

}

void CalibrationItem::CheckCurrentState()
{
    qDebug() << "--IN--CheckCurrentState";

    m_cali_stop = false;

    double  value = 0;
    int32   codeCount = 0;
    int32   adjustCode[RELATED_CNT_MAX] = {0};
    //QString str_cali_state = "xxxxxx";

    int32   tableRowCount = m_subj_list->getCount();    

    CaliState caliState = CaliIdle;

    do{
        if(m_matching_ai_auto || m_matching_ao_auto){
            for(int32 i = 0; i < tableRowCount; i++)
            {
                caliState = m_subj_list->getItem(i).AdjCodeGetCurrentState(&value, &codeCount, adjustCode);
                qDebug() << "--TimerTicked--Row[" << i << "], Cali State[" << caliState << "], Value:" << value << ", Codde Count:" << codeCount << ", Adjust Code = " << adjustCode[0];
                QThread::msleep(10);

                emit update_state_signal(i, caliState);                
            }
        }
        if(m_matching_ai_manual || m_matching_ao_manual){
            qDebug() << "--IN--ManualCali";
            caliState = m_subj_list->getItem(m_subj_idx).AdjCodeGetCurrentState(&value, &codeCount, adjustCode);
            QThread::msleep(500);
            //int32   idx = codeCount - 1;
            emit update_val_signal(value);
            //ui.lineEdit_read_value->setText(QString::number(value));


        }

    }while(!m_cali_stop);

    qDebug() << "--OUT--CheckCurrentState";
}

void CalibrationItem::SettingWidgetsState()
{        
    m_strPtr = m_sect_list->getItem(m_sect_idx).getTypeName();

    //Matching is 0; Mis-matching is 1    
    if(m_matching_ai_manual || m_matching_ao_manual){
        ui.lbl_1->setVisible(true);
        ui.lbl_2->setVisible(true);
        ui.lbl_3->setVisible(true);
        ui.lbl_4->setVisible(true);
        ui.lbl_5->setVisible(true);
        ui.cmb_subj_desc->setVisible(true);
        ui.textEdit_subj_instrc->setVisible(true);
        ui.lbl_subj_target->setVisible(true);
        ui.lineEdit_read_value->setVisible(true);
        ui.spb_adj_code->setVisible(true);
        ui.table_list_result->setVisible(false);

        ui.btn_config->setVisible(true);
        ui.btn_start->setVisible(true);
        ui.btn_stop->setVisible(true);
        ui.btn_save->setVisible(true);        
    }

    if(m_matching_ai_auto || m_matching_ao_auto){
        ui.lbl_1->setVisible(false);
        ui.lbl_2->setVisible(false);
        ui.lbl_3->setVisible(false);
        ui.lbl_4->setVisible(false);
        ui.lbl_5->setVisible(false);
        ui.cmb_subj_desc->setVisible(false);
        ui.textEdit_subj_instrc->setVisible(false);
        ui.lbl_subj_target->setVisible(false);
        ui.lineEdit_read_value->setVisible(false);
        ui.spb_adj_code->setVisible(false);
        ui.table_list_result->setVisible(true);

        ui.btn_config->setVisible(true);
        ui.btn_start->setVisible(true);
        ui.btn_stop->setVisible(true);
        ui.btn_save->setVisible(true);
    }
}

void CalibrationItem::SectionTypeCheck()
{
    m_matching_ai_auto   = 0;
    m_matching_ai_manual = 0;

    m_matching_ao_auto   = 0;
    m_matching_ao_manual = 0;

    m_strPtr = m_sect_list->getItem(m_sect_idx).getTypeName();    
    qDebug("m_str_buf[%ls]\n", m_strPtr);

    m_matching_ai_auto = !(QString::compare(QString::fromWCharArray(m_strPtr), "Cali.Section.InSignals.Batch"));
    m_matching_ai_manual |= !(QString::compare(QString::fromWCharArray(m_strPtr), "Cali.Section.InSignals.DigitalVR"));
    //m_matching_ai_manual |= !(QString::compare(QString::fromWCharArray(m_strPtr), "Cali.Section.InSignals.KnobVR"));

    m_matching_ao_auto = !(QString::compare(QString::fromWCharArray(m_strPtr), "Cali.Section.OutSignals.Batch"));
    m_matching_ao_manual |= !(QString::compare(QString::fromWCharArray(m_strPtr), "Cali.Section.OutSignals.DigitalVR"));
    //m_matching_ao_manual |= !(QString::compare(QString::fromWCharArray(m_strPtr), "Cali.Section.OutSignals.KnobVR"));
}

void CalibrationItem::CheckError(ErrorCode errorCode)
{
    if (BioFailed(errorCode))
	{
		QString message = tr("Sorry, there are some errors occurred, Error Code: 0x") +
			QString::number(errorCode, 16).right(8).toUpper();
		QMessageBox::information(this, "Warning Information", message);
	}
}

void CalibrationItem::on_update_state(int idx, CaliState state)
{
    CaliState caliState = state;
    int lastOneIdx = m_subj_list->getCount() - 1;

    switch(caliState)
    {
    case CaliSuccess:   m_str_state = "Success";    break;
    case CaliFailed:    m_str_state = "Failed";     break;
    case CaliBusy:      m_str_state = "Running";    break;
    default:            m_str_state = "---NA---";     break;
    }

    if(!m_subjCaliDone[idx]){
        ui.table_list_result->setItem(idx, 2, new QTableWidgetItem(m_str_state));

        if(caliState == CaliSuccess || caliState == CaliFailed){
            m_subjCaliDone[idx] = 1;
        }

        if(caliState == CaliFailed || (idx == lastOneIdx) && (caliState == CaliSuccess || caliState == CaliFailed)){
            ui.btn_save->setEnabled(true);
            ButtonStopClicked();            
            m_cali_stop = true;
            m_thread.terminate();
            qDebug() << "Auto Cali Finish~~";
        }
    }
}

void CalibrationItem::on_update_value(int value)
{    
    ui.lineEdit_read_value->setText(QString::number(value));
}
