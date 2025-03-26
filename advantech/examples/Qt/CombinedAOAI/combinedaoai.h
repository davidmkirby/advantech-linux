#ifndef COMBINEDAOAI_H
#define COMBINEDAOAI_H

#include <QDialog>
#include <QTimer>
#include <QButtonGroup>
#include "configuredialog.h"
#include "../common/simplegraph.h"
#include "../common/WaveformGenerator.h"

namespace Ui {
class CombinedAOAI;
}

class CombinedAOAI : public QDialog
{
    Q_OBJECT

public:
    CombinedAOAI(QDialog *parent = nullptr, Qt::WindowFlags flags = {});
    ~CombinedAOAI();

    void Initialize();
    void SetConfigureDialog(ConfigureDialog *dialog){configureDialog = dialog;}
    void SetConfigureParameter(ConfigureParameter value){configure = value;}

private:
    // Common methods
    void ConfigureDevice();
    void CheckError(ErrorCode errorCode);

    // AI related methods
    void ConfigureAI();
    void ConfigureAIGraph();
    void InitializeAIList();
    void SetXCord();

    // AO related methods
    void ConfigureAO();
    void ConfigureAOPanel();

private:
    Ui::CombinedAOAI *ui;
    ConfigureDialog *configureDialog;
    ConfigureParameter configure;

    // AI related members
    WaveformAiCtrl *waveformAiCtrl;
    double *scaledData;
    int rawDataBufferLength;
    TimeUnit timeUnit;
    double xInc;
    SimpleGraph *graph;

    // AO related members
    InstantAoCtrl *instantAoCtrl;
    int aoChannelStart;
    int aoChannelCount;
    double dataScaled[2];
    WaveformParameter m_waveParam[2];
    WaveformGenerator *waveformGenerator;
    QPushButton* buttons[6];
    int m_wavePtIdx[2];
    bool m_waveSeled[2];
    QTimer *timer;
    QString strs[6];
    QButtonGroup *buttonGroup1;
    QButtonGroup *buttonGroup2;

private slots:
    // Common slots
    void ButtonConfigureClicked();

    // AI related slots
    void DivValueChanged(int value);
    void ButtonAIStartClicked();
    void ButtonAIPauseClicked();
    void ButtonAIStopClicked();

    // AO related slots
    void TimerTicked();
    void SliderValueChanged(int value);
    void ManualClicked(int id);
    void WaveButtonClicked(int id);

    // AI Event handlers
    static void BDAQCALL OnDataReadyEvent(void *sender, BfdAiEventArgs *args, void *userParam);
    static void BDAQCALL OnOverRunEvent(void *sender, BfdAiEventArgs *args, void *userParam);
    static void BDAQCALL OnCacheOverflowEvent(void *sender, BfdAiEventArgs *args, void *userParam);
    static void BDAQCALL OnStoppedEvent(void *sender, BfdAiEventArgs *args, void *userParam);
};

#endif // COMBINEDAOAI_H
