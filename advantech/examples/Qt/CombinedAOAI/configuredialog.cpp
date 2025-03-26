#include "configuredialog.h"
#include "ui_configuredialog.h"
#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>

#define MAXCLOCKRATE 500000000

ConfigureDialog::ConfigureDialog(QDialog *parent)
    : QDialog(parent),
    ui(new Ui::ConfigureDialog)
{
    ui->setupUi(this);

    //Set the minimum and close button of the main frame.
    this->setWindowFlags(Qt::WindowFlags(Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint));

    connect(ui->cmbAIDevice, SIGNAL(currentIndexChanged(int)), this, SLOT(AIDeviceChanged(int)));
    connect(ui->cmbAODevice, SIGNAL(currentIndexChanged(int)), this, SLOT(AODeviceChanged(int)));
    connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(ButtonOKClicked()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(ButtonCancelClicked()));
    connect(ui->btnAIBrowse, SIGNAL(clicked()), this, SLOT(AIButtonBrowseClicked()));
    connect(ui->btnAOBrowse, SIGNAL(clicked()), this, SLOT(AOButtonBrowseClicked()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(TabChanged(int)));

    //Set the maximum value of clock rate per channel 500MHz = 500000000.
    ui->edtClockRatePerChan->setValidator(new QDoubleValidator(1, MAXCLOCKRATE, 2, this));
    ui->txtPointCount->setValidator(new QIntValidator(0, 10000000, this));

    Initialization();
}

ConfigureDialog::~ConfigureDialog()
{
    delete ui;
}

void ConfigureDialog::Initialization()
{
    // Check for AI devices - PCIE-1816 only
    WaveformAiCtrl *waveformAiCtrl = WaveformAiCtrl::Create();
    Array<DeviceTreeNode> *supportedAiDevices = waveformAiCtrl->getSupportedDevices();

    // Check for AO devices - Both PCIE-1816 and PCIE-1824
    InstantAoCtrl *instantAoCtrl = InstantAoCtrl::Create();
    Array<DeviceTreeNode> *supportedAoDevices = instantAoCtrl->getSupportedDevices();

    // Handle case where no supported devices are found
    if (supportedAiDevices->getCount() == 0 && supportedAoDevices->getCount() == 0)
    {
        QMessageBox::information(this, tr("Warning Information"),
            tr("No device to support the currently demonstrated functions!"));
        QCoreApplication::quit();
    }
    else
    {
        // Initialize AI device dropdown (PCIE-1816)
        ui->cmbAIDevice->blockSignals(true);
        for (int i = 0; i < supportedAiDevices->getCount(); i++)
        {
            DeviceTreeNode const &node = supportedAiDevices->getItem(i);
            QString description = QString::fromWCharArray(node.Description);
            // Filter for PCIE-1816 or add any AI capable device
            if (description.contains("PCIE-1816", Qt::CaseInsensitive)) {
                ui->cmbAIDevice->addItem(description);
            }
        }
        ui->cmbAIDevice->blockSignals(false);

        // Initialize AO device dropdown (PCIE-1816 and PCIE-1824)
        ui->cmbAODevice->blockSignals(true);
        for (int i = 0; i < supportedAoDevices->getCount(); i++)
        {
            DeviceTreeNode const &node = supportedAoDevices->getItem(i);
            QString description = QString::fromWCharArray(node.Description);
            // Add both PCIE-1816 and PCIE-1824
            if (description.contains("PCIE-1816", Qt::CaseInsensitive) ||
                description.contains("PCIE-1824", Qt::CaseInsensitive)) {
                ui->cmbAODevice->addItem(description);
            }
        }
        ui->cmbAODevice->blockSignals(false);

        // Set initial selections if devices are available
        if (ui->cmbAIDevice->count() > 0) {
            ui->cmbAIDevice->setCurrentIndex(0);
            AIDeviceChanged(0);
            ui->tabWidget->setTabEnabled(0, true);
        } else {
            ui->tabWidget->setTabEnabled(0, false);
        }

        if (ui->cmbAODevice->count() > 0) {
            ui->cmbAODevice->setCurrentIndex(0);
            AODeviceChanged(0);
            ui->tabWidget->setTabEnabled(1, true);
        } else {
            ui->tabWidget->setTabEnabled(1, false);
        }
    }

    waveformAiCtrl->Dispose();
    supportedAiDevices->Dispose();
    instantAoCtrl->Dispose();
    supportedAoDevices->Dispose();
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

void ConfigureDialog::AIDeviceChanged(int index)
{
    if (index < 0 || ui->cmbAIDevice->count() == 0) return;

    ui->aiCmbChannelCount->clear();
    ui->aiCmbChannelStart->clear();
    ui->aiCmbValueRange->clear();

    std::wstring description = ui->cmbAIDevice->currentText().toStdWString();
    DeviceInformation selected(description.c_str());

    WaveformAiCtrl *waveformAiCtrl = WaveformAiCtrl::Create();
    ErrorCode errorCode = waveformAiCtrl->setSelectedDevice(selected);

    if (errorCode == Success)
    {
        // Device supports AI
        int channelCount = (waveformAiCtrl->getChannelCount() < 16) ?
            waveformAiCtrl->getChannelCount() : 16;
        int logicChannelCount = waveformAiCtrl->getChannelCount();

        for (int i = 0; i < logicChannelCount; i++)
        {
            ui->aiCmbChannelStart->addItem(QString("%1").arg(i));
        }

        for (int i = 0; i < channelCount; i++)
        {
            ui->aiCmbChannelCount->addItem(QString("%1").arg(i + 1));
        }

        Array<ValueRange> *aiValueRanges = waveformAiCtrl->getFeatures()->getValueRanges();
        wchar_t vrgDescription[128];
        MathInterval ranges;
        ValueUnit valueUnit;

        for(int i = 0; i < aiValueRanges->getCount(); i++)
        {
            errorCode = AdxGetValueRangeInformation(aiValueRanges->getItem(i),
                sizeof(vrgDescription), vrgDescription, &ranges, &valueUnit);
            CheckError(errorCode);

            QString str = QString::fromWCharArray(vrgDescription);
            ui->aiCmbValueRange->addItem(str);
        }

        // Set default AI values
        ui->aiCmbChannelStart->setCurrentIndex(0);
        ui->aiCmbChannelCount->setCurrentIndex(1);
        ui->aiCmbValueRange->setCurrentIndex(0);

        ui->tabWidget->setTabEnabled(0, true);
    }
    else
    {
        ui->tabWidget->setTabEnabled(0, false);
    }

    ui->btnOK->setEnabled(ui->tabWidget->isTabEnabled(0) || ui->tabWidget->isTabEnabled(1));

    // If the current tab is disabled, switch to an enabled tab
    if (!ui->tabWidget->isTabEnabled(ui->tabWidget->currentIndex()))
    {
        if (ui->tabWidget->isTabEnabled(0))
            ui->tabWidget->setCurrentIndex(0);
        else if (ui->tabWidget->isTabEnabled(1))
            ui->tabWidget->setCurrentIndex(1);
    }

    waveformAiCtrl->Dispose();
}

void ConfigureDialog::AODeviceChanged(int index)
{
    if (index < 0 || ui->cmbAODevice->count() == 0) return;

    ui->aoCmbChannelCount->clear();
    ui->aoCmbChannelStart->clear();
    ui->aoCmbValueRange->clear();

    std::wstring description = ui->cmbAODevice->currentText().toStdWString();
    DeviceInformation selected(description.c_str());

    InstantAoCtrl *instantAoCtrl = InstantAoCtrl::Create();
    ErrorCode errorCode = instantAoCtrl->setSelectedDevice(selected);

    if (errorCode == Success)
    {
        // Device supports AO
        int channelCount = (instantAoCtrl->getChannelCount() < 2) ?
            instantAoCtrl->getChannelCount() : 2;
        int logicChannelCount = instantAoCtrl->getChannelCount();

        for (int i = 0; i < logicChannelCount; i++)
        {
            ui->aoCmbChannelStart->addItem(QString("%1").arg(i));
        }

        for (int i = 0; i < channelCount; i++)
        {
            ui->aoCmbChannelCount->addItem(QString("%1").arg(i + 1));
        }

        Array<ValueRange> *aoValueRanges = instantAoCtrl->getFeatures()->getValueRanges();
        wchar_t vrgDescription[128];
        MathInterval ranges;

        for (int i = 0; i < aoValueRanges->getCount(); i++)
        {
            if (aoValueRanges->getItem(i) < UserCustomizedVrgStart) {
                errorCode = AdxGetValueRangeInformation(aoValueRanges->getItem(i),
                    sizeof(vrgDescription), vrgDescription, &ranges, NULL);
                CheckError(errorCode);
                QString str = QString::fromWCharArray(vrgDescription);
                ui->aoCmbValueRange->addItem(str);
            }
        }

        // Set default AO values
        ui->aoCmbChannelStart->setCurrentIndex(0);
        ui->aoCmbChannelCount->setCurrentIndex(1);
        ui->aoCmbValueRange->setCurrentIndex(0);

        ui->tabWidget->setTabEnabled(1, true);
    }
    else
    {
        ui->tabWidget->setTabEnabled(1, false);
    }

    ui->btnOK->setEnabled(ui->tabWidget->isTabEnabled(0) || ui->tabWidget->isTabEnabled(1));

    // If the current tab is disabled, switch to an enabled tab
    if (!ui->tabWidget->isTabEnabled(ui->tabWidget->currentIndex()))
    {
        if (ui->tabWidget->isTabEnabled(0))
            ui->tabWidget->setCurrentIndex(0);
        else if (ui->tabWidget->isTabEnabled(1))
            ui->tabWidget->setCurrentIndex(1);
    }

    instantAoCtrl->Dispose();
}

void ConfigureDialog::TabChanged(int index)
{
    // Adjust window height based on active tab
    // This is optional but can make the dialog look cleaner
}

void ConfigureDialog::ButtonOKClicked()
{
    if (ui->cmbAIDevice->count() == 0 && ui->cmbAODevice->count() == 0)
    {
        QCoreApplication::quit();
    }

    // Get AI settings if available
    if (ui->tabWidget->isTabEnabled(0))
    {
        configure.aiDeviceName = ui->cmbAIDevice->currentText();
        configure.aiProfilePath = ui->txtAIProfilePath->text();

        std::wstring description = ui->cmbAIDevice->currentText().toStdWString();
        DeviceInformation selected(description.c_str());

        WaveformAiCtrl *waveformAiCtrl = WaveformAiCtrl::Create();
        ErrorCode errorCode = waveformAiCtrl->setSelectedDevice(selected);
        CheckError(errorCode);

        Array<ValueRange> *aiValueRanges = waveformAiCtrl->getFeatures()->getValueRanges();
        configure.aiChannelCount = ui->aiCmbChannelCount->currentText().toInt();
        configure.aiChannelStart = ui->aiCmbChannelStart->currentText().toInt();
        configure.aiValueRange = aiValueRanges->getItem(ui->aiCmbValueRange->currentIndex());
        configure.clockRatePerChan = ui->edtClockRatePerChan->text().toDouble();
        configure.sectionLength = ui->edtSectionLength->text().toInt();

        waveformAiCtrl->Dispose();
    }
    else
    {
        // Default AI settings if not available
        configure.aiDeviceName = "";
        configure.aiChannelCount = 0;
    }

    // Get AO settings if available
    if (ui->tabWidget->isTabEnabled(1))
    {
        configure.aoDeviceName = ui->cmbAODevice->currentText();
        configure.aoProfilePath = ui->txtAOProfilePath->text();

        std::wstring description = ui->cmbAODevice->currentText().toStdWString();
        DeviceInformation selected(description.c_str());

        InstantAoCtrl *instantAoCtrl = InstantAoCtrl::Create();
        ErrorCode errorCode = instantAoCtrl->setSelectedDevice(selected);
        CheckError(errorCode);

        Array<ValueRange> *aoValueRanges = instantAoCtrl->getFeatures()->getValueRanges();
        configure.aoChannelCount = ui->aoCmbChannelCount->currentText().toInt();
        configure.aoChannelStart = ui->aoCmbChannelStart->currentText().toInt();
        configure.aoValueRange = aoValueRanges->getItem(ui->aoCmbValueRange->currentIndex());
        configure.pointCountPerWave = ui->txtPointCount->text().toInt();

        instantAoCtrl->Dispose();
    }
    else
    {
        // Default AO settings if not available
        configure.aoDeviceName = "";
        configure.aoChannelCount = 0;
    }

    this->accept();
}

void ConfigureDialog::ButtonCancelClicked()
{
    this->reject();
}

void ConfigureDialog::AIButtonBrowseClicked()
{
    QString str = QFileDialog::getOpenFileName(this, tr("Open AI Profile"), "../../profile", tr("Image Files(*.xml)"));
    ui->txtAIProfilePath->setText(str);
    configure.aiProfilePath = str;
}

void ConfigureDialog::AOButtonBrowseClicked()
{
    QString str = QFileDialog::getOpenFileName(this, tr("Open AO Profile"), "../../profile", tr("Image Files(*.xml)"));
    ui->txtAOProfilePath->setText(str);
    configure.aoProfilePath = str;
}

void ConfigureDialog::RefreshConfigureParameter()
{
    // Refresh AI parameters if available
    if (!configure.aiDeviceName.isEmpty())
    {
        std::wstring description = configure.aiDeviceName.toStdWString();
        DeviceInformation selected(description.c_str());

        WaveformAiCtrl *waveformAiCtrl = WaveformAiCtrl::Create();
        ErrorCode errorCode = waveformAiCtrl->setSelectedDevice(selected);
        if (errorCode == Success)
        {
            ui->edtClockRatePerChan->setText(QString::number(waveformAiCtrl->getConversion()->getClockRate(), 'f', 0));
            ui->edtSectionLength->setText(QString::number(waveformAiCtrl->getRecord()->getSectionLength(), 'f', 0));
        }
        waveformAiCtrl->Dispose();
    }
}