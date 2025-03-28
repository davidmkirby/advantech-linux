#include "streamingao.h"
#include <QMessageBox>

StreamingAO::StreamingAO(QWidget *parent, Qt::WindowFlags flags)
	: QDialog(parent, flags)
{
	ui.setupUi(this);

	//Set the minimum and close button of the main frame.
	this->setWindowFlags(Qt::WindowFlags(Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint));

    bfdAoCtrl = BufferedAoCtrl::Create();
    dataScaled = NULL;
	this->m_waveSeled[0] = false;
	this->m_waveSeled[1] = false;

    bfdAoCtrl->addStoppedHandler(OnStoppedEvent, this);
    bfdAoCtrl->addUnderrunHandler(OnUnderrunEvent, this);
	
    this->buttonGroup1 = new QButtonGroup();
	this->buttonGroup1->addButton(ui.BtnSineA, 0);
	this->buttonGroup1->addButton(ui.BtnTriangleA, 1);
	this->buttonGroup1->addButton(ui.BtnSquareA, 2);
	this->buttonGroup1->addButton(ui.BtnSineB, 3);
	this->buttonGroup1->addButton(ui.BtnTriangleB, 4);
	this->buttonGroup1->addButton(ui.BtnSquareB, 5);
	this->buttonGroup1->setExclusive(false);

	this->buttons[0] = ui.BtnSineA;
	this->buttons[1] = ui.BtnTriangleA;
	this->buttons[2] = ui.BtnSquareA;
	this->buttons[3] = ui.BtnSineB;
	this->buttons[4] = ui.BtnTriangleB;
	this->buttons[5] = ui.BtnSquareB;
	
	strs[0] = "background:url(:/StreamingAO/Resources/sine.png)";
	strs[1] = "background:url(:/StreamingAO/Resources/sine_down.png)";
	strs[2] = "background:url(:/StreamingAO/Resources/triangle.png)";
	strs[3] = "background:url(:/StreamingAO/Resources/triangle_down.png)";
	strs[4] = "background:url(:/StreamingAO/Resources/square.png)";
	strs[5] = "background:url(:/StreamingAO/Resources/square_down.png)";

	connect(ui.btnConfigure, SIGNAL(clicked()), this, SLOT(ButtonConfigureClicked()));
	connect(ui.btnStart, SIGNAL(clicked()), this, SLOT(ButtonStartClicked()));
	connect(buttonGroup1, SIGNAL(buttonClicked(int)), this, SLOT(WaveButtonClicked(int)));
	connect(this, SIGNAL(stophandle()), this, SLOT(stopped()));
	connect(this, SIGNAL(underrunhandle()), this, SLOT(underrunned()));
}

StreamingAO::~StreamingAO()
{

}

void StreamingAO::Initialize() {
	//set the title of the form.
	this->setWindowTitle(tr("Streaming AO - Run(") + configure.deviceName + tr(")"));
	
	if (dataScaled != NULL) {
        delete[] dataScaled;
        dataScaled = NULL;
    }

	waveformGenerator = new WaveformGenerator(configure.pointCountPerWave);

	InitialButtons();

	//initialize the sine as the default choice
	ui.BtnSineA->setChecked(true);
	ui.BtnSineA->setStyleSheet(strs[1]);
	m_waveSeled[0] = true;
	m_waveParam[0].Style = sine;

	if (configure.channelCount > 1) {
		ui.BtnSineB->setChecked(true);
		ui.BtnSineB->setStyleSheet(strs[1]);
		m_waveSeled[1] = true;
		m_waveParam[1].Style = sine;
	}

	ConfigureDevice();

	//set the ChannelCount
    if (bfdAoCtrl->getScanChannel()->getChannelCount() > 1) {
        bfdAoCtrl->getScanChannel()->setChannelCount(2);
    }

	ConfigurePanel();
}

void StreamingAO::ConfigureDevice() {
	ErrorCode errorCode = Success;

    std::wstring description = configure.deviceName.toStdWString();
    DeviceInformation selected(description.c_str());

	errorCode = bfdAoCtrl->setSelectedDevice(selected);
	CheckError(errorCode);

    std::wstring profile = configure.profilePath.toStdWString();
    errorCode = bfdAoCtrl->LoadProfile(profile.c_str());
    CheckError(errorCode);

	errorCode = bfdAoCtrl->setStreaming(true);
    CheckError(errorCode);

    ScanChannel* scanChannel = bfdAoCtrl->getScanChannel();
    errorCode = scanChannel->setChannelStart(configure.channelStart);
    CheckError(errorCode);
    errorCode = scanChannel->setChannelCount(configure.channelCount);
    CheckError(errorCode);
    errorCode = scanChannel->setSamples(configure.pointCountPerWave);
    CheckError(errorCode);
    errorCode = scanChannel->setIntervalCount(configure.pointCountPerWave);
    CheckError(errorCode);
    errorCode = bfdAoCtrl->getConvertClock()->setRate(configure.pointCountPerWave * configure.clockRate);
    CheckError(errorCode);

   for (int i = 0; i < bfdAoCtrl -> getChannels() -> getCount(); i++) {
       errorCode = bfdAoCtrl -> getChannels() -> getItem(i).setValueRange(configure.valueRange);
       CheckError(errorCode);
   }
}

void StreamingAO::CheckError(ErrorCode errorCode) {
    if (BioFailed(errorCode)) {
		QString message = tr("Sorry, there are some errors occurred, Error Code: 0x") +
		QString::number(errorCode, 16).right(8).toUpper();
		QMessageBox::information(this, "Warning Information", message);
	}
}

void StreamingAO::ButtonConfigureClicked() {
	int dialogResult = configDialog->exec();
	if (dialogResult == QDialog::Accepted) {
		configure = configDialog->GetConfigureParameter();
		Initialize();
	}
}

void StreamingAO::ConfigurePanel() {
	if (configure.channelCount == 1) {
		ui.BtnSineB->setEnabled(false);
		ui.BtnTriangleB->setEnabled(false);
		ui.BtnSquareB->setEnabled(false);
		ui.txtboxHiLevelB->setEnabled(false);
		ui.txtboxLoLevelB->setEnabled(false);
		ui.chLabelB->setText(tr(""));
	} else {
		ui.BtnSineB->setEnabled(true);
		ui.BtnTriangleB->setEnabled(true);
		ui.BtnSquareB->setEnabled(true);
		ui.txtboxHiLevelB->setEnabled(true);
		ui.txtboxLoLevelB->setEnabled(true);
		ui.chLabelB->setText(QString("%1").arg((configure.channelStart + 1) % bfdAoCtrl->getChannelCount()));
	}
	ui.chLabelA->setText(QString("%1").arg(configure.channelStart));
}

void StreamingAO::InitialButtons() {
	for (int i = 0; i < 6; i++) {
		buttons[i]->setChecked(false);
		buttons[i]->setStyleSheet(strs[(i % 3) * 2]);
	}
}

void StreamingAO::OnStoppedEvent(void * sender , BfdAoEventArgs *args, void * userParam)
{
   StreamingAO *uParam = (StreamingAO *)userParam;
   uParam->stophandle();
}

void StreamingAO::OnUnderrunEvent(void * sender , BfdAoEventArgs *args, void * userParam)
{
   StreamingAO *uParam = (StreamingAO *)userParam;
   uParam->underrunhandle();
}

void StreamingAO::stopped() {
	QMessageBox::information(this, "Warning Information", "EvtBufferedAOStopped!");
}

void StreamingAO::underrunned() {
    //QMessageBox::information(this, "Warning Information", "EvtBufferedAOUnderrun!");
}

void StreamingAO::WaveButtonClicked(int id) {
	int seledCH, baseIdx, imgIdx;

	seledCH = id / 3;
	baseIdx = seledCH * 3;

	for (int i = baseIdx; i < baseIdx + 3; i++) {
		if (i != id && buttons[i]->isChecked()) {
			imgIdx = (i- baseIdx) * 2;
			buttons[i]->setChecked(false);
			buttons[i]->setStyleSheet(strs[imgIdx]);
		}
	}

	if (buttons[id]->isChecked()) {
		buttons[id]->setStyleSheet(strs[(id - baseIdx) * 2 + 1]);
	} else {
		buttons[id]->setStyleSheet(strs[(id - baseIdx) * 2]);
	}

	m_waveSeled[seledCH] = buttons[id]->isChecked();

	if (buttons[id]->isChecked()) {
		m_waveParam[seledCH].Style = (WaveformStyle)(id % 3);
	} else {
		m_waveParam[seledCH].Style = (WaveformStyle)2;
	}
}

void StreamingAO::ButtonStartClicked() {
	int dataCount;
	ErrorCode errorCode = Success;

	if (ui.btnStart->text() == "Start") {
		if ((m_waveSeled[0] == false) && (m_waveSeled[1] == false)) {
			QMessageBox::information(this, "Warning Information", "Please select a waveform!");
			return;
		}

		//prepare the AO
        errorCode = bfdAoCtrl->Prepare();
        CheckError(errorCode);

		ScanChannel* scanChannel = bfdAoCtrl->getScanChannel();
        dataCount = scanChannel->getChannelCount() * scanChannel->getSamples();
		dataScaled = new double[dataCount];

		if ((m_waveSeled[0] == true) && (m_waveSeled[1] == false)) {
			m_waveParam[0].HighLevel = ui.txtboxHiLevelA->text().toDouble();
            m_waveParam[1].HighLevel = 0;
			m_waveParam[0].LowLevel = ui.txtboxLoLevelA->text().toDouble();
            m_waveParam[1].LowLevel = 0;
        } else if ((m_waveSeled[0] == false) && (m_waveSeled[1] == true)) {
            m_waveParam[0].HighLevel = 0;
			m_waveParam[1].HighLevel = ui.txtboxHiLevelB->text().toDouble();
            m_waveParam[0].LowLevel = 0;
			m_waveParam[1].LowLevel = ui.txtboxLoLevelB->text().toDouble();
        } else if ((m_waveSeled[0] == true) && (m_waveSeled[1] == true)) {
            m_waveParam[0].HighLevel = ui.txtboxHiLevelA->text().toDouble();
            m_waveParam[1].HighLevel = ui.txtboxHiLevelB->text().toDouble();
            m_waveParam[0].LowLevel = ui.txtboxLoLevelA->text().toDouble();
            m_waveParam[1].LowLevel = ui.txtboxLoLevelB->text().toDouble();
        }

		//generator the wave
		if (configure.channelCount <= 1) {
			this->waveformGenerator->GetOnePeriod(dataScaled, m_waveParam[0]);
		} else {
			this->waveformGenerator->GetMultipleWaveform(dataScaled, 2, m_waveParam);
		}

		//Convert scaled  data to raw data
		errorCode = bfdAoCtrl->SetData(dataCount, dataScaled);
        CheckError(errorCode);

        errorCode = bfdAoCtrl->Start();
        CheckError(errorCode);

		//disable the checkBox and the textBox
		ui.btnStart->setText(tr("Stop"));

		ui.txtboxHiLevelA->setEnabled(false);
		ui.txtboxHiLevelB->setEnabled(false);
		ui.txtboxLoLevelA->setEnabled(false);
		ui.txtboxLoLevelB->setEnabled(false);

		for (int i = 0; i < 6; i++) {
			buttons[i]->setEnabled(false);
		}

		ui.btnConfigure->setEnabled(false);
	} else {
		ui.btnStart->setText(tr("Start"));
		
		errorCode = bfdAoCtrl->Stop(0);
        CheckError(errorCode);

		ui.txtboxHiLevelA->setEnabled(true);
		ui.txtboxHiLevelB->setEnabled(true);
		ui.txtboxLoLevelA->setEnabled(true);
		ui.txtboxLoLevelB->setEnabled(true);

		for (int i = 0; i < 6; i++) {
			buttons[i]->setEnabled(true);
		}

		ui.btnConfigure->setEnabled(true);
	}
}
