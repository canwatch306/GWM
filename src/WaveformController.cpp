#include "WaveformController.h"

#include <QFile>
#include <QTextStream>
#include <cstdio>  
#include <cstdlib> 
#include <QFileDialog>
#include <QTextCodec>
#include <QVector>
#include <fstream>
#include <cstdint>
#include <QDebug>
#include <thread>
#include <algorithm>

bool flag = true;

int count = 0;



WaveformController::WaveformController(WaveformModel* model, WaveformView* view)
    : m_model(model), m_view(view), m_vmusbwave(new VmUsbWave()){
    connect(m_vmusbwave, &VmUsbWave::updateDatas, this, &WaveformController::processData);
   
    buffer_ch1 = new double[vmDevice.vmDos.depth*1024];
    std::fill(buffer_ch1, buffer_ch1 + vmDevice.vmDos.depth*1024, 0);
}

WaveformController::~WaveformController() {
    cleanupDevice();
    delete m_vmusbwave;
    m_vmusbwave = nullptr; // Avoid dangling pointer
        delete [] buffer_ch1;
     buffer_ch1 = nullptr;
}

void WaveformController::initDevice() {
    count = vmDevice.vmDds.count;
    qDebug() << "DDS output";
    int chanelID = vmDevice.channleID;
    unsigned long long loopsNums = (unsigned long long) vmDevice.vmDds.cycles;
    unsigned long long periodNs = (unsigned long long) vmDevice.vmDds.intervals * 1e6;
    unsigned int ddsFre = (unsigned int)vmDevice.vmDds.frequency;
    unsigned long long burstDelay = ((double)loopsNums*0.02/ddsFre)* 1e9;
    int amplitudeMv = vmDevice.vmDds.peak;
    int preTrigger = vmDevice.vmDos.preSamplDepth;
    WaveForm waveStyle = vmDevice.vmDds.wave;
    SetAcDc(chanelID,0);
    SetOscSample(vmDevice.vmDos.samplRate);
    SetOscChannelRange(chanelID,-3000,3000);
    int realSize = Capture(vmDevice.vmDos.depth,0x0001,0);
    SetTriggerMode(TRIGGER_MODE_LIANXU);
    SetTriggerStyle(TRIGGER_STYLE_RISE_EDGE);
    SetTriggerSource(TRIGGER_SOURCE_LOGIC0);
    SetPreTriggerPercent(preTrigger);
    IOEnable(chanelID,1);
    SetIOInOut(chanelID,0);
    SetTriggerLevel(1500);
    qDebug() << "realSize "<< realSize;
    SetDDSOutMode(chanelID, DDS_OUT_MODE_BURST);
    SetDDSBurstStyle( chanelID, 0);
    SetDDSLoopsNum(chanelID,loopsNums);
    SetDDSBurstDelayNs(chanelID,burstDelay);
    SetDDSBurstPeriodNs(chanelID,periodNs);
    SetDDSTriggerSource(chanelID, DDS_TRIGGER_SOURCE_INTERNAL);
    SetDDSPinlv(chanelID, ddsFre);
    SetDDSAmplitudeMv(chanelID, amplitudeMv);
    SetDDSBoxingStyle(chanelID, waveStyle);
    IOEnable(1,1);
    SetIOInOut(1,1);
    SetIOOutState(1,4);
    SetDDSOutputGateEnge(chanelID,1);
    DDSOutputEnable(chanelID, 1);
}

void WaveformController::ddsDeviceInit() {
    initDevice(); 

}


void WaveformController::cleanupDevice() {
    qDebug() << "Cleaning up device...";
    //m_vmusbwave->captureEnable(false);
}

void WaveformController::startDataCollection() {
    qDebug() << "Starting data collection...";
    flag = true;
    ddsDeviceInit();   
    
    //Call to update device ID after initialization
    // processData(); // Collect data once and stop
    // stopDataCollection(); // Stop the data collection after processing
}

void WaveformController::stopDataCollection() {
    qDebug() << "Stopping data collection...";
    DDSOutputEnable(0, 0);
    SetTriggerMode(TRIGGER_MODE_AUTO);
    flag = false;
    //m_vmusbwave->captureEnable(false);
}

void WaveformController::processData() {
   
    if (!m_vmusbwave || !flag || !count) {
        qWarning() << "VmUsbWave instance is not initialized.";
        std::fill(buffer_ch1, buffer_ch1 + vmDevice.vmDos.depth*1024, 0);
        if (!count)
        emit captureUpdated();
        return;
    }
    const int bufferSize = vmDevice.vmDos.depth; // 2048K samples
    QVector<double> xData(bufferSize*1024);
    QVector<double> yData(bufferSize*1024);

    double *buffer_ch1_temp = new double[bufferSize*1024];
    int length_ch1 = ReadVoltageDatas(0, buffer_ch1_temp, bufferSize*1024);
   
    double sampleFrequency = double(vmDevice.vmDos.samplRate); // Get sample frequency
    //qDebug() <<sampleFrequency<<" ";
    double timeStep = 1000.0 / sampleFrequency; // Convert to milliseconds
    for (int i = 0; i < bufferSize*1024; ++i) {
        xData[i] = i * timeStep;
        // if (count != vmDevice.vmDds.count)
        // buffer_ch1[i] = (buffer_ch1[i] + buffer_ch1_temp[i])/2.0;
        // else
        buffer_ch1[i] = buffer_ch1_temp[i];
        yData[i] = buffer_ch1[i];
    }
    m_model->setData(xData, yData);

    // Auto-adjust the axes based on the data
    //m_view->autoAdjustAxes();
    m_view->update();
    count--;
    qDebug() <<"current count"<<count;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    Capture(vmDevice.vmDos.depth,0x0001,0);
}

void WaveformController::loadData(const QString& fileName) {
    if (fileName.isEmpty() ) {
        qWarning() << "File name is empty.";
        return;
    }

    QTextCodec* codec = QTextCodec::codecForName("GB2312");
    QByteArray encodedName = codec->fromUnicode(fileName);
    std::ifstream file1(encodedName.data(), std::ios::binary);

    if (!file1) {
        qWarning() << "Failed to open file.";
        return;
    }

    file1.read(reinterpret_cast<char*>(&FileHead), sizeof(FILEHEAD));

    if (FileHead.version != 0.02) {
        file1.close();
        file1.open(encodedName.data(), std::ios::binary);
        if (!file1) {
            qWarning() << "Failed to reopen file.";
            return;
        }
        file1.seekg(64, std::ios::beg);

        int sample_frequency_1 = 5;
        int sample_time_1 = 20;
        int voltage_data_length = static_cast<int>(sample_frequency_1 * sample_time_1 * 1000);
        std::vector<uint16_t> ch1_voltage_data_array(voltage_data_length);

        file1.read(reinterpret_cast<char*>(ch1_voltage_data_array.data()), voltage_data_length * sizeof(uint16_t));
        file1.close();

        QVector<double> xData(voltage_data_length);
        QVector<double> yData(voltage_data_length);

        for (int i = 0; i < voltage_data_length; ++i) {
            xData[i] = static_cast<double>(i) / (sample_frequency_1 * 1.0e6) * 1e3; // Convert to milliseconds
            yData[i] = static_cast<double>((ch1_voltage_data_array[i] - 32768.0) / 32768.0 * 8); // Convert to voltage
        }

        m_model->setData(xData, yData);
    }
}

void WaveformController::updateDeviceID() {
    if (m_vmusbwave) {
         uint64_t id = (static_cast<uint64_t>(GetOnlyId1()) << 32) | GetOnlyId0();
        QString deviceID =  QString::number(id); // Replace with actual method call
        emit deviceIDUpdated(deviceID); // Emit signal to update UI
    }
}