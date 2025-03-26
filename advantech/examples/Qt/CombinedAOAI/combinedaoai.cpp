#include "combinedaoai.h"
#include "ui_combinedaoai.h"
#include <QMessageBox>
#include <QPalette>
#include <qmath.h>
#include <QButtonGroup>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QListWidget>
#include <QBrush>

CombinedAOAI::CombinedAOAI(QDialog *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
    ui(new Ui::CombinedAOAI)
{
    ui->setupUi(this);

    //Set the minimum and close button of the main frame.
    this->setWindowFlags(Qt::WindowFlags(Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint));

    // Initialize AI components
    graph = new SimpleGraph(ui->graphFrame);
    graph->setFixedSize(ui->graphFrame->size());

    timeUnit = Millisecond;
    rawDataBufferLength = 0;
    scaledData = NULL;

    waveformAiCtrl = WaveformAiCtrl::Create();
    waveformAiCtrl->addDataReadyHandler(OnDataReadyEvent, this);
    waveformAiCtrl->addOverrunHandler(OnOverRunEvent, this);
    waveformAiCtrl->addCacheOverflowHandler(OnCacheOverflowEvent, this);
    waveformAiCtrl->addStoppedHandler(OnStoppedEvent, this);

    // Initialize AO components
    this->buttonGroup1 = new QButtonGroup();
    this->buttonGroup1->addButton(ui->BtnSineA, 0);
    this->buttonGroup1->addButton(ui->BtnTriangleA, 1);
    this->buttonGroup1->addButton(ui->BtnSquareA, 2);
    this->buttonGroup1->addButton(ui->BtnSineB, 3);
    this->buttonGroup1->addButton(ui->BtnTriangleB, 4);
    this->buttonGroup1->addButton(ui->BtnSquareB, 5);
    this->buttonGroup1->setExclusive(false);

    this->buttonGroup2 = new QButtonGroup();
    this->buttonGroup2->addButton(ui->BtnManualA, 0);
    this->buttonGroup2->addButton(ui->BtnManualB, 1);
    this->buttonGroup2->setExclusive(false);

    this->buttons[0] = ui->BtnSineA;
    this->buttons[1] = ui->BtnTriangleA;
    this->buttons[2] = ui->BtnSquareA;
    this->buttons[3] = ui->BtnSineB;
    this->buttons[4] = ui->BtnTriangleB;
    this->buttons[5] = ui->BtnSquareB;

    strs[0] = "background:url(:/AO_AI/Resources/sine.png)";
    strs[1] = "background:url(:/AO_AI/Resources/sine_down.png)";
    strs[2] = "background:url(:/AO_AI/Resources/triangle.png)";
    strs[3] = "background:url(:/AO_AI/Resources/triangle_down.png)";
    strs[4] = "background:url(:/AO_AI/Resources/square.png)";
    strs[5] = "background:url(:/AO_AI/Resources/square_down.png)";

    instantAoCtrl = InstantAoCtrl::Create();
    this->m_waveSeled[0] = false;
    this->m_waveSeled[1] = false;

    timer = new QTimer(this);

    // Connect signals and slots
    // AI connections
    connect(ui->btnConfigure, &QPushButton::clicked, this, &CombinedAOAI::ButtonConfigureClicked);
    connect(ui->btnAIStart, &QPushButton::clicked, this, &CombinedAOAI::ButtonAIStartClicked);
    connect(ui->btnAIPause, &QPushButton::clicked, this, &CombinedAOAI::ButtonAIPauseClicked);
    connect(ui->btnAIStop, &QPushButton::clicked, this, &CombinedAOAI::ButtonAIStopClicked);
    connect(ui->sldDiv, &QSlider::valueChanged, this, &CombinedAOAI::DivValueChanged);

    // AO connections
    connect(timer, &QTimer::timeout, this, &CombinedAOAI::TimerTicked);
    connect(ui->timerTrackBar, &QSlider::valueChanged, this, &CombinedAOAI::SliderValueChanged);
    connect(this->buttonGroup1, &QButtonGroup::idClicked, this, &CombinedAOAI::WaveButtonClicked);
    connect(this->buttonGroup2, &QButtonGroup::idClicked, this, &CombinedAOAI::ManualClicked);
}

CombinedAOAI::~CombinedAOAI()
{
    // Clean up AI components
    if (waveformAiCtrl != NULL)
    {
        waveformAiCtrl->Dispose();
        waveformAiCtrl = NULL;
    }

    if (scaledData != NULL)
    {
        delete []scaledData;
        scaledData = NULL;
    }

    if (graph != NULL)
    {
        delete graph;
        graph = NULL;
    }

    // Clean up AO components
    if (instantAoCtrl != NULL)
    {
        instantAoCtrl->Dispose();
        instantAoCtrl = NULL;
    }

    if (timer != NULL)
    {
        timer->stop();
    }

    delete ui;
}

void CombinedAOAI::Initialize()
{
    QString title = "Combined AI/AO - Run";
    if (!configure.aiDeviceName.isEmpty() && !configure.aoDeviceName.isEmpty()) {
        title += "(AI: " + configure.aiDeviceName + ", AO: " + configure.aoDeviceName + ")";
    } else if (!configure.aiDeviceName.isEmpty()) {
        title += "(AI: " + configure.aiDeviceName + ")";
    } else if (!configure.aoDeviceName.isEmpty()) {
        title += "(AO: " + configure.aoDeviceName + ")";
    }
    setWindowTitle(title);

    ConfigureDevice();

    // Set active tabs based on configured devices
    ui->tabWidget->setTabEnabled(0, !configure.aiDeviceName.isEmpty());
    ui->tabWidget->setTabEnabled(1, !configure.aoDeviceName.isEmpty());

    if (!configure.aiDeviceName.isEmpty()) {
        ui->tabWidget->setCurrentIndex(0);
    } else if (!configure.aoDeviceName.isEmpty()) {
        ui->tabWidget->setCurrentIndex(1);
    }

    // Initialize AI components if enabled
    if (!configure.aiDeviceName.isEmpty()) {
        ConfigureAIGraph();
        InitializeAIList();

        ui->btnAIStart->setEnabled(true);
        ui->btnAIPause->setEnabled(false);
        ui->btnAIStop->setEnabled(false);
        ui->sldDiv->setEnabled(true);
    } else {
        ui->btnAIStart->setEnabled(false);
        ui->btnAIPause->setEnabled(false);
        ui->btnAIStop->setEnabled(false);
        ui->sldDiv->setEnabled(false);
    }

    // Initialize AO components if enabled
    if (!configure.aoDeviceName.isEmpty()) {
        waveformGenerator = new WaveformGenerator(configure.pointCountPerWave);
        ConfigureAOPanel();

        // Initialize a timer which drives the data acquisition.
        timer->start(50);
    }

    ui->btnConfigure->setEnabled(true);
}

void CombinedAOAI::ConfigureDevice()
{
    // Configure both AI and AO using separate devices
    ConfigureAI();
    ConfigureAO();
}

void CombinedAOAI::CheckError(ErrorCode errorCode)
{
    if (BioFailed(errorCode))
    {
        QString message = tr("Sorry, there are some errors occurred, Error Code: 0x") +
            QString::number(errorCode, 16).right(8).toUpper();
        QMessageBox::information(this, "Warning Information", message);
    }
}

// AI Methods
void CombinedAOAI::ConfigureAI()
{
    if (configure.aiDeviceName.isEmpty()) return;

    ErrorCode errorCode = Success;
    if (scaledData != NULL)
    {
        delete []scaledData;
        scaledData = NULL;
    }

    rawDataBufferLength = configure.sectionLength * configure.aiChannelCount;

    scaledData = new double[rawDataBufferLength];
    if (scaledData == NULL)
    {
        QMessageBox::information(this, tr("Warning Information"),
            tr("Sorry! Error in allocating memory...."));
        return;
    }

    // Select the AI device (PCIE-1816)
    std::wstring description = configure.aiDeviceName.toStdWString();
    DeviceInformation selected(description.c_str());

    errorCode = waveformAiCtrl->setSelectedDevice(selected);
    CheckError(errorCode);

    if (!configure.aiProfilePath.isEmpty()) {
        std::wstring profile = configure.aiProfilePath.toStdWString();
        errorCode = waveformAiCtrl->LoadProfile(profile.c_str());
        CheckError(errorCode);
    }

    // Set the streaming mode
    errorCode = waveformAiCtrl->getConversion()->setChannelCount(configure.aiChannelCount);
    CheckError(errorCode);
    errorCode = waveformAiCtrl->getConversion()->setChannelStart(configure.aiChannelStart);
    CheckError(errorCode);
    errorCode = waveformAiCtrl->getConversion()->setClockRate(configure.clockRatePerChan);
    CheckError(errorCode);
    errorCode = waveformAiCtrl->getRecord()->setSectionLength(configure.sectionLength);
    CheckError(errorCode);
    errorCode = waveformAiCtrl->getRecord()->setSectionCount(0); // Setting '0' means streaming mode
    CheckError(errorCode);

    for (int i = 0; i < waveformAiCtrl->getChannels()->getCount(); i++)
    {
        errorCode = waveformAiCtrl->getChannels()->getItem(i).setValueRange(configure.aiValueRange);
        CheckError(errorCode);
    }

    errorCode = waveformAiCtrl->Prepare();
    CheckError(errorCode);
}

void CombinedAOAI::ConfigureAIGraph()
{
    double clockRate = waveformAiCtrl->getConversion()->getClockRate();
    int tUnit = (int)Millisecond;
    double timeInterval = 100.0 * graph->rect().width() / clockRate;
    while (clockRate >= 10 * 1000)
    {
        timeInterval *= 1000;
        clockRate /= 1000;
        --tUnit;
    }
    timeUnit = (TimeUnit)tUnit;

    int divValue = (int)timeInterval;
    ui->sldDiv->setMaximum(4 * divValue);
    int divMin = divValue / 10;
    if (divMin == 0)
    {
        divMin = 1;
    }
    ui->sldDiv->setMinimum(divMin);
    ui->sldDiv->setValue(divValue);
    SetXCord();

    ValueUnit unit;
    MathInterval rangeY;
    QString yRanges[3];
    ErrorCode errorCode = AdxGetValueRangeInformation(configure.aiValueRange, 0, NULL,
        &rangeY, &unit);
    CheckError(errorCode);

    graph->GetYCordRange(yRanges, rangeY.Max, rangeY.Min, unit);
    ui->lblYCoordinateMax->setText(yRanges[0]);
    ui->lblYCoordinateMin->setText(yRanges[1]);
    ui->lblYCoordinateMid->setText(yRanges[2]);

    graph->m_yCordRangeMax = rangeY.Max;
    graph->m_yCordRangeMin = rangeY.Min;
    graph->Clear();
}

void CombinedAOAI::InitializeAIList()
{
    ui->listWidget->clear();
    QListWidgetItem *item = NULL;
    for (int i = 0; i < configure.aiChannelCount; i++)
    {
        item = new QListWidgetItem(tr(""), ui->listWidget);
        item->setBackground(SimpleGraph::lineColor[i]);
        item->setSizeHint(QSize(54, 21));
    }
}

void CombinedAOAI::SetXCord()
{
    graph->m_xCordTimeDiv = (double)ui->sldDiv->value();
    QString xRanges[2];

    double shiftMaxValue = qRound(graph->m_xCordTimeDiv * 10 + graph->m_xCordTimeOffset);
    graph->GetXCordRange(xRanges, shiftMaxValue, graph->m_xCordTimeOffset, timeUnit);
    ui->lblXCoordinateStart->setText(xRanges[1]);
    ui->lblXCoordinateEnd->setText(xRanges[0]);
}

void CombinedAOAI::DivValueChanged(int value)
{
    graph->Div(value);
    SetXCord();
}

void CombinedAOAI::ButtonAIStartClicked()
{
    ui->btnConfigure->setEnabled(false);
    ui->btnAIStart->setEnabled(false);
    ui->btnAIPause->setEnabled(true);
    ui->btnAIStop->setEnabled(true);

    ErrorCode errorCode = waveformAiCtrl->Start();
    CheckError(errorCode);
    xInc = 1.0 / waveformAiCtrl->getConversion()->getClockRate();
}

void CombinedAOAI::ButtonAIPauseClicked()
{
    ErrorCode errorCode = waveformAiCtrl->Stop();
    CheckError(errorCode);

    ui->btnAIStart->setEnabled(true);
    ui->btnAIPause->setEnabled(false);
    ui->btnAIStop->setEnabled(true);
}

void CombinedAOAI::ButtonAIStopClicked()
{
    ErrorCode errorCode = waveformAiCtrl->Stop();
    CheckError(errorCode);

    graph->Clear();

    ui->btnConfigure->setEnabled(true);
    ui->btnAIStart->setEnabled(true);
    ui->btnAIPause->setEnabled(false);
    ui->btnAIStop->setEnabled(false);
}

// AO Methods
void CombinedAOAI::ConfigureAO()
{
    if (configure.aoDeviceName.isEmpty()) return;

    ErrorCode errorCode = Success;

    // Select the AO device (PCIE-1816 or PCIE-1824)
    std::wstring description = configure.aoDeviceName.toStdWString();
    DeviceInformation selected(description.c_str());

    errorCode = instantAoCtrl->setSelectedDevice(selected);
    CheckError(errorCode);

    if (!configure.aoProfilePath.isEmpty()) {
        std::wstring profile = configure.aoProfilePath.toStdWString();
        errorCode = instantAoCtrl->LoadProfile(profile.c_str());
        CheckError(errorCode);
    }

    for (int i = 0; i < instantAoCtrl->getChannels()->getCount(); i++) {
        errorCode = instantAoCtrl->getChannels()->getItem(i).setValueRange(configure.aoValueRange);
        CheckError(errorCode);
    }
}

void CombinedAOAI::ConfigureAOPanel()
{
    if (configure.aoChannelCount == 1) {
        aoChannelStart = configure.aoChannelStart;
        aoChannelCount = 1;

        ui->BtnSineB->setEnabled(false);
        ui->BtnTriangleB->setEnabled(false);
        ui->BtnSquareB->setEnabled(false);
        ui->BtnManualB->setEnabled(false);
        ui->txtboxHiLevelB->setEnabled(false);
        ui->txtboxLoLevelB->setEnabled(false);
        ui->txtboxValueB->setEnabled(false);
        ui->chLabelB->setText(tr(""));
    } else {
        aoChannelStart = configure.aoChannelStart;
        aoChannelCount = 2;

        ui->BtnSineB->setEnabled(true);
        ui->BtnTriangleB->setEnabled(true);
        ui->BtnSquareB->setEnabled(true);
        ui->BtnManualB->setEnabled(true);
        ui->txtboxHiLevelB->setEnabled(true);
        ui->txtboxLoLevelB->setEnabled(true);
        ui->txtboxValueB->setEnabled(true);
        ui->chLabelB->setText(QString("%1").arg((aoChannelStart + 1) % instantAoCtrl->getChannelCount()));
    }
    ui->chLabelA->setText(QString("%1").arg(aoChannelStart));
}

void CombinedAOAI::SliderValueChanged(int)
{
    timer->setInterval(ui->timerTrackBar->value());
    ui->timerFreqLabel->setText(QString("%1").arg(ui->timerTrackBar->value()) + tr("ms"));
}

void CombinedAOAI::WaveButtonClicked(int id)
{
    int seledCH, baseIdx, imgIdx;

    seledCH = id / 3;
    baseIdx = seledCH * 3;

    for (int i = baseIdx; i < baseIdx + 3; i++) {
        if (i != id && buttons[i]->isChecked()) {
            imgIdx = (i - baseIdx) * 2;
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
    m_waveParam[seledCH].Style = (WaveformStyle)(id % 3);

    if (seledCH == 0) {
        m_waveParam[seledCH].HighLevel = ui->txtboxHiLevelA->text().toDouble();
        m_waveParam[seledCH].LowLevel = ui->txtboxLoLevelA->text().toDouble();
    } else {
        m_waveParam[seledCH].HighLevel = ui->txtboxHiLevelB->text().toDouble();
        m_waveParam[seledCH].LowLevel = ui->txtboxLoLevelB->text().toDouble();
    }
    m_wavePtIdx[seledCH] = 0;
}

void CombinedAOAI::ManualClicked(int id)
{
    int ch, baseIdx;

    // Manual output button click
    ch = id;
    baseIdx = id * 3;

    for (int i = 0; i < 3; i++) {
        if (buttons[i + baseIdx]->isChecked()) {
            buttons[i + baseIdx]->setChecked(false);
            buttons[i + baseIdx]->setStyleSheet(strs[i * 2]);
        }
    }

    m_waveSeled[ch] = false;

    if (ch == 0) {
        dataScaled[ch] = ui->txtboxValueA->text().toDouble();
    } else {
        dataScaled[ch] = ui->txtboxValueB->text().toDouble();
    }
}

void CombinedAOAI::TimerTicked()
{
    if (configure.aoDeviceName.isEmpty()) return;

    for (int i = 0; i < 2; i++) {
        if (m_waveSeled[i]) {
            dataScaled[i] = waveformGenerator->GetOnePoint(m_waveParam[i].Style, m_wavePtIdx[i], m_waveParam[i].HighLevel, m_waveParam[i].LowLevel);
            m_wavePtIdx[i] = (m_wavePtIdx[i] + 1) % configure.pointCountPerWave;
        }
    }

    ErrorCode errorCode = Success;
    errorCode = instantAoCtrl->Write(aoChannelStart, aoChannelCount, dataScaled);
    if (errorCode != Success) {
        timer->stop();
        CheckError(errorCode);
    }
}

void CombinedAOAI::ButtonConfigureClicked()
{
    // Stop AI if it's running
    if (!configure.aiDeviceName.isEmpty() && ui->btnAIPause->isEnabled()) {
        ButtonAIPauseClicked();
    }

    // Stop AO timer temporarily
    if (!configure.aoDeviceName.isEmpty()) {
        timer->stop();
    }

    int dialogResult = configureDialog->exec();
    if (dialogResult == QDialog::Accepted) {
        configure = configureDialog->GetConfigureParameter();
        Initialize();
    } else {
        // Restart the timer if dialog was canceled
        if (!configure.aoDeviceName.isEmpty()) {
            timer->start();
        }
    }
}

// AI Event handlers
void BDAQCALL CombinedAOAI::OnDataReadyEvent(void * sender, BfdAiEventArgs * args, void * userParam)
{
    CombinedAOAI * uParam = (CombinedAOAI *)userParam;
    int32 remainingCount = args->Count;
    int32 getDataCount = 0, returnedCount = 0;
    int32 bufSize = uParam->configure.sectionLength * uParam->configure.aiChannelCount;

    do {
        getDataCount = qMin(bufSize, remainingCount);
        ErrorCode ret = ((WaveformAiCtrl*)sender)->GetData(getDataCount, uParam->scaledData, 0, &returnedCount, NULL, NULL, NULL);
        remainingCount -= returnedCount;
        if (ret != Success && ret != WarningRecordEnd)
        {
            QString message = QObject::tr("Sorry, there are some errors occurred, Error Code: 0x") +
                QString::number(ret, 16).right(8);
            QMessageBox::information(uParam, "Warning Information", message);
            return;
        }
	
	// Display the data on the graph	
        uParam->graph->Chart(uParam->scaledData, uParam->configure.aiChannelCount, returnedCount / uParam->configure.aiChannelCount, uParam->xInc);
        
        // Pass-through to AO - if AO is enabled and we have valid data
        if (!uParam->configure.aoDeviceName.isEmpty() && uParam->ui->chkPassThrough->isChecked() && returnedCount > 0) {
			// Get the last sample from each channel to send to AO
			for (int i = 0; i < qMin(uParam->configure.aiChannelCount, uParam->aoChannelCount); i++) {
				uParam->dataScaled[i] = uParam->scaledData[returnedCount - uParam->configure.aiChannelCount + i];
			}
			
			// Write to AO
			ErrorCode aoRet = uParam->instantAoCtrl->Write(uParam->aoChannelStart, uParam->aoChannelCount, uParam->dataScaled);
			if (aoRet != Success) {
				// Handle AO write error if needed
			}
		}
    } while(remainingCount > 0);
}

void BDAQCALL CombinedAOAI::OnOverRunEvent(void * sender, BfdAiEventArgs * args, void * userParam)
{
    // For debug/logging, commented out for production code
    /*
    CombinedAOAI * uParam = (CombinedAOAI *)userParam;
    QString message = QObject::tr("The event over run has happened!");
    QMessageBox::information(uParam, "Warning Information", message);
    */
}

void BDAQCALL CombinedAOAI::OnCacheOverflowEvent(void * sender, BfdAiEventArgs * args, void * userParam)
{
    // For debug/logging, commented out for production code
    /*
    CombinedAOAI * uParam = (CombinedAOAI *)userParam;
    QString message = QObject::tr("The event cache over flow has happened!");
    QMessageBox::information(uParam, "Warning Information", message);
    */
}

void BDAQCALL CombinedAOAI::OnStoppedEvent(void * sender, BfdAiEventArgs * args, void * userParam)
{
    // For debug/logging, commented out for production code
    /*
    CombinedAOAI * uParam = (CombinedAOAI *)userParam;
    QString message = QObject::tr("The event stopped has happened!");
    QMessageBox::information(uParam, "Warning Information", message);
    */
}
