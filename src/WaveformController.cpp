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
#include <QThread>
#include <algorithm>

WaveformController::WaveformController(WaveformModel* model, WaveformView* view)
    : m_model(model), m_view(view), m_vmusbwave(new VmUsbWave()) {
    initDevice();
    
    
}

WaveformController::~WaveformController() {
    cleanupDevice();
    delete m_vmusbwave;
    m_vmusbwave = nullptr; // Avoid dangling pointer
}

void WaveformController::initDevice() {
    qDebug() << "Initializing device...";
    m_vmusbwave->setsample(5000000); // Set sample rate (example)
    m_vmusbwave->setTriggerMode(0);  // Set trigger mode (example)
    m_vmusbwave->captureEnable(true);
    m_vmusbwave->setCaptureLength(64); // Set to 2048K samples
    m_vmusbwave->setTriggerStyle(0);
    m_vmusbwave->setTriggerSource(0);
    m_vmusbwave->setTriggerLevel(0);
}

void WaveformController::cleanupDevice() {
    qDebug() << "Cleaning up device...";
    m_vmusbwave->captureEnable(false);
}

void WaveformController::startDataCollection() {
    qDebug() << "Starting data collection...";
        
    //Call to update device ID after initialization
    processData(); // Collect data once and stop
    stopDataCollection(); // Stop the data collection after processing
}

void WaveformController::stopDataCollection() {
    qDebug() << "Stopping data collection...";
    m_vmusbwave->captureEnable(false);
}

void WaveformController::processData() {
    if (!m_vmusbwave) {
        qWarning() << "VmUsbWave instance is not initialized.";
        return;
    }

    const int bufferSize = 64; // 2048K samples
    QVector<double> xData(bufferSize);
    QVector<double> yData(bufferSize);

    double *buffer_ch1 = new double[bufferSize];
    Capture(bufferSize, 0x0001, 1);
    int length_ch1 = ReadVoltageDatas(0, buffer_ch1, bufferSize);
   
    double sampleFrequency = m_vmusbwave->getsample(); // Get sample frequency
    double timeStep = 1.0 / sampleFrequency * 1e3; // Convert to milliseconds

    for (int i = 0; i < length_ch1; ++i) {
        xData[i] = i * timeStep;
        yData[i] = (buffer_ch1[i] - 32768.0) / 32768.0 * 8; // Convert to voltage
         qDebug() <<yData[i]<<" ";
    }

    delete [] buffer_ch1;
    buffer_ch1 = nullptr;
    m_model->setData(xData, yData);

    // Auto-adjust the axes based on the data
    m_view->autoAdjustAxes();
}

void WaveformController::loadData(const QString& fileName) {
    if (fileName.isEmpty()) {
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