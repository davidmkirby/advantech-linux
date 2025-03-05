#ifndef CALIBATIONITEM_H
#define CALIBATIONITEM_H

#include <QDialog>
#include <QTimer>
#include <QThread>
#include "ui_calibrationitem.h"
#include "configuredialog.h"

class CalibrationItem : public QDialog
{
	Q_OBJECT

public:
    CalibrationItem(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~CalibrationItem();
	void Initialize();
	void ConfigureDevice();
	void CheckError(ErrorCode errorCode);
	void SetConfigureDialog(ConfigureDialog *dialog){this->configDialog = dialog;}
	void SetConfigureParameter(ConfigureParameter value){this->configure = value;}
    void InitTableList();
    void CheckCurrentState();

private:
   class CaliThread : public QThread
   {
   public:
      CaliThread(CalibrationItem *owner) : m_owner(*owner) { }
      virtual void run() { m_owner.CheckCurrentState(); }

    public:
      CalibrationItem &m_owner;
   };

private:
    Ui::CalibrationItemClass ui;
    CalibrationCtrl* m_caliCtrl;
	ConfigureDialog *configDialog;
    ConfigureParameter configure;
    CaliThread         m_thread;

    wchar_t const *    m_strPtr;
    QString            m_str_state;

    bool    m_matching_ai_auto;
    bool    m_matching_ai_manual;
    bool    m_matching_ao_auto;
    bool    m_matching_ao_manual;
    bool    m_cali_stop;

    int     m_sln_idx;
    int     m_sect_idx;
    int     m_subj_idx;

    int32   m_subjCaliDone[512];
    Array<CaliSolution>* m_sln_list;
    Array<CaliSection>*  m_sect_list;
    Array<CaliSubject>*  m_subj_list;    

    QTimer* timer;
    QTimer* timer1;

    void SectionChangeForWidgetsClear();
    void SubjectChangeForWidgetsClear();    
    void SettingWidgetsState();
    void SectionTypeCheck();

signals:
    void update_state_signal(int idx, CaliState state);
    void update_val_signal(int value);

private slots:
	void ButtonStartClicked();
	void ButtonStopClicked();
	void ButtonConfigureClicked();    

    void ButtonSaveClicked();
    void SpinBoxClicked();    

    void onCmbSect();
    void onCmbSubj();

    void TimerTicked();
    void TimerTicked1();

    void on_update_state(int idx, CaliState state);
    void on_update_value(int value);
};

#endif // CALIBATIONITEM_H
