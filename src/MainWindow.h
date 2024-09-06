#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QSlider>
#include <QPixmap>
#include <QString>
#include <QSpacerItem>
#include <QGridLayout>
#include "WaveformModel.h"
#include "WaveformView.h"
#include "WaveformController.h"
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    
    ~MainWindow();


private slots:
    void on_loadDataButton_clicked();
    void chooseLineColor();
    void updateAxisRanges();  // Slot to update axis ranges
    void updateXAxisRange(int value);
    void updateYAxisRange(int value);
    void startDataCollection(); // New slot for starting data collection
    void stopDataCollection();  // New slot for stopping data collection
    void toggleButton();
    void setConnectionStatus(bool connected);

public slots:
    void updateDeviceID(const QString& id);

public slots:
    void monitorConnectionStatus(bool connected);


private:
/***********设备初始化************/
    QLabel *deviceID;
    QWidget* centralWidget;
    QWidget* otherTips;
    QVBoxLayout* tipsLayout;
    QVBoxLayout* mainLayout;
    QHBoxLayout* buttonLayout;
    QGridLayout *inputLayout;  // Layout for input fields and sliders
    QPushButton* loadDataButton;
    QPushButton* colorButton;
    QPushButton* autoAdjustButton;
    QPushButton* startCollectionButton; // New button for starting data collection
    QPushButton* stopCollectionButton;  // New button for stopping data collection
    QHBoxLayout* xRangeLayout;
    QLineEdit *xRangeInput;
    QHBoxLayout* yRangeLayout;
    QLineEdit *yRangeInput;
    QLabel *xLabel;
    QLabel *yLabel;
    QSlider *xSlider;  // X-axis slider
    QSlider *ySlider;  // Y-axis slider
    WaveformModel* m_model;
    WaveformView* m_view;
    WaveformController* m_controller;
    
    QLabel *connectionLabel;
    QLabel *statusIconLabel;
    QLabel *actStatusLabel;
    QPushButton *actButton;
    bool isRunning;
    QString startButtonIconPath;
    QString stopButtonIconPath;
    QString statusTextRunning;
    QString statusTextStopped;


    QLineEdit *peakValueInput;
    QLineEdit *frequencyInput;
    QComboBox *waveformComboBox; 
    QLineEdit *countInput;
    QLineEdit *periodInput;
    QLineEdit *intervalInput;

    QLineEdit *sampleInput;
    QLineEdit *captureLengthInput;
    QHBoxLayout* paramLayout;
    QHBoxLayout *connectionLayout;
    QVBoxLayout *actbuttonLayout;
    QGridLayout *excitationLayout;
    QHBoxLayout *receptionLayout;
    QGridLayout *filterLayout;
    
    bool dataCollectionActive; // Flag to indicate if data collection is active
};

#endif // MAINWINDOW_H
