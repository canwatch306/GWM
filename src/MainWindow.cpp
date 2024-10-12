#include "mainwindow.h"
#include <QFileDialog>
#include <QWidget>
#include <QColorDialog>
#include <QDebug>
#include <QSplitter>
#include <QScreen>
#include <QTimer>

#define ICONSCALE  100
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    centralWidget(new QWidget(this)),
    otherTips(new QWidget(this)),
    mainLayout(new QVBoxLayout(centralWidget)),
    tipsLayout(new QVBoxLayout(otherTips)),
    buttonLayout(new QHBoxLayout),
    inputLayout(new QGridLayout),
    loadDataButton(new QPushButton("文件读取", this)),
    colorButton(new QPushButton("线形", this)),
    autoAdjustButton(new QPushButton("自适应", this)),
    startCollectionButton(new QPushButton("开始采集", this)),
    stopCollectionButton(new QPushButton("停止采集", this)),
    xRangeInput(new QLineEdit(this)),
    yRangeInput(new QLineEdit(this)),
    xLabel(new QLabel("时间:", this)),
    yLabel(new QLabel("电压:", this)),
    deviceID(new QLabel("设备ID:", this)),
    xSlider(new QSlider(Qt::Horizontal, this)),
    ySlider(new QSlider(Qt::Horizontal, this)),
    m_model(new WaveformModel),
    m_view(new WaveformView(this)),
    m_controller(new WaveformController(m_model, m_view)),
    isRunning(true),
    startButtonIconPath(":/icons/start.png"),
    stopButtonIconPath(":/icons/stop.png"),
    statusTextRunning("开始"),
    statusTextStopped("停止"),
    dataCollectionActive(false)
{
    // Initialize the splitter and central widget
    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(centralWidget);
    splitter->addWidget(m_view);
    splitter->addWidget(otherTips);

    splitter->setStretchFactor(1, 20); // Give more space to the waveform view
    setCentralWidget(splitter);

    // Set minimum size for the main window
    setMinimumSize(800, 1200);

    // Configure buttons
    QPushButton *buttons[] = {loadDataButton, colorButton, autoAdjustButton, startCollectionButton, stopCollectionButton};
    for (QPushButton *button : buttons) {
        button->setFixedSize(150, 60);
    }

    // Configure sliders
    xSlider->setRange(0, 200);
    ySlider->setRange(0, 200);

    // Set up buttonLayout
    buttonLayout->addWidget(loadDataButton);
    buttonLayout->addWidget(colorButton);
    buttonLayout->addWidget(autoAdjustButton);
    buttonLayout->addWidget(startCollectionButton);
    buttonLayout->addWidget(stopCollectionButton);
    buttonLayout->setStretch(0, 1);
    buttonLayout->setStretch(1, 1);
    buttonLayout->setStretch(2, 1);
    buttonLayout->setStretch(3, 1);
    buttonLayout->setStretch(4, 1);

    // Create and configure connectionLayout
    statusIconLabel = new QLabel(this);
    setConnectionStatus(false);
    connectionLayout = new QHBoxLayout();
    connectionLayout->addWidget(statusIconLabel);

    // Create and configure actbuttonLayout
    actButton = new QPushButton(this);
    QIcon icon(":/icons/stop.png");
    if (icon.isNull()) {
        qDebug() << "Failed to load icon:" << ":/icons/stop.png";
    } else {
        actButton->setIcon(icon);
        actButton->setIconSize(QSize(ICONSCALE, ICONSCALE));
        actButton->setFixedSize(QSize(ICONSCALE, ICONSCALE));
        actButton->setStyleSheet(
            "QPushButton {"
            "  background: transparent;"
            "  border: none;"
            "}"
        );
    }
    connect(actButton, &QPushButton::clicked, this, &MainWindow::toggleButton);
    
    actStatusLabel = new QLabel(statusTextRunning);
    actStatusLabel->setAlignment(Qt::AlignCenter);
    actbuttonLayout = new QVBoxLayout();
    actbuttonLayout->addWidget(actButton);
    actbuttonLayout->addWidget(actStatusLabel);
    actbuttonLayout->setAlignment(Qt::AlignCenter);
    excitationLayout = new QGridLayout;
    // Create and configure excitationLayout
    QLabel *peakValueLabel = new QLabel("幅值(V)");
    peakValueInput = new QLineEdit();
    peakValueInput->setMaximumWidth(100);

    // Set up a validator to ensure the input is a float with up to 3 decimals  
    QDoubleValidator *validatorPeak = new QDoubleValidator(this);  
    validatorPeak->setNotation(QDoubleValidator::StandardNotation);  
    validatorPeak->setDecimals(3); // Set the maximum number of decimals to 3  
    peakValueInput->setValidator(validatorPeak); 
    connect(peakValueInput, &QLineEdit::editingFinished, this, &MainWindow::onInputPeakValue);


    QLabel *frequencyLabel  = new QLabel("频率(KHZ)");
    frequencyInput = new QLineEdit();
    frequencyInput->setMaximumWidth(100);
    QIntValidator *validatorFre = new QIntValidator(1, 5000, this);  
    frequencyInput->setValidator(validatorFre); 
    connect(frequencyInput, &QLineEdit::editingFinished, this, &MainWindow::onInputFreValue);

    QLabel *waveformLabel = new QLabel("波形");
    waveformComboBox = new QComboBox();
    waveformComboBox->addItems({"Arb","Sine", "Square", "Triangle"});
    waveformComboBox->setMaximumWidth(100);


    QLabel *countLabel = new QLabel("重复次数");
    countInput = new QLineEdit();
    countInput->setMaximumWidth(100);
    QIntValidator *validatorLoops = new QIntValidator(1, 200, this);  
    countInput->setValidator(validatorLoops); 
    connect(countInput, &QLineEdit::editingFinished, this, &MainWindow::onInputLoopNumValue);

    QLabel *periodLabel = new QLabel("周期数(s)");
    periodInput = new QLineEdit();
    periodInput->setMaximumWidth(100);
    QIntValidator *validatorCycles = new QIntValidator(1, 20, this);  
    periodInput->setValidator(validatorCycles); 
    connect(periodInput, &QLineEdit::editingFinished, this, &MainWindow::onInputCyclesValue);

    QLabel *intervalLabel = new QLabel("间隔(ms)");
    intervalInput = new QLineEdit();
    intervalInput->setMaximumWidth(100);
    QIntValidator *validatorInterval = new QIntValidator(1, 1000, this);  
    intervalInput->setValidator(validatorInterval); 
    connect(intervalInput, &QLineEdit::editingFinished, this, &MainWindow::onInputPearidValue);

    QHBoxLayout *rowPeak = new QHBoxLayout;
    rowPeak->addWidget(peakValueLabel);
    rowPeak->addWidget(peakValueInput);
    rowPeak->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum));

    QHBoxLayout *rowFre = new QHBoxLayout;
    rowFre->addWidget(frequencyLabel);
    rowFre->addWidget(frequencyInput);
    rowFre->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum));

    QHBoxLayout *rowWaveForm = new QHBoxLayout;
    rowWaveForm->addWidget(waveformLabel);
    rowWaveForm->addWidget(waveformComboBox);
    rowWaveForm->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum));

    QHBoxLayout *rowCount = new QHBoxLayout;
    rowCount->addWidget(countLabel);
    rowCount->addWidget(countInput);
    rowCount->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum));
    QHBoxLayout *rowPeriod = new QHBoxLayout;
    rowPeriod->addWidget(periodLabel);
    rowPeriod->addWidget(periodInput);
    rowPeriod->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum));

    QHBoxLayout *rowInterval = new QHBoxLayout;
    rowInterval->addWidget(intervalLabel);
    rowInterval->addWidget(intervalInput);
    rowInterval->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum));

    excitationLayout->addLayout(rowPeak, 0, 0);
    excitationLayout->addLayout(rowFre, 1, 0);
    excitationLayout->addLayout(rowWaveForm, 2, 0);
    excitationLayout->addLayout(rowCount, 0, 1);
    excitationLayout->addLayout(rowPeriod, 1, 1);
    excitationLayout->addLayout(rowInterval, 2, 1);
    excitationLayout->setHorizontalSpacing(20); // 固定水平间距

    receptionLayout = new QHBoxLayout();
    QLabel *sampleInputLabel = new QLabel("采样率(M)");
    sampleInput = new QLineEdit();
    sampleInput->setFixedWidth(100);
    QDoubleValidator *validatorSample = new QDoubleValidator(this);  
    validatorSample->setNotation(QDoubleValidator::StandardNotation);  
    validatorSample->setDecimals(3); // Set the maximum number of decimals to 3  
    sampleInput->setValidator(validatorSample); 
    connect(sampleInput, &QLineEdit::editingFinished, this, &MainWindow::onInputSampleValue);

    QLabel *captureLengthLabel = new QLabel("时间(ms)");
    captureLengthInput = new QLineEdit();
    captureLengthInput->setFixedWidth(100);
    QDoubleValidator *validatorDepth = new QDoubleValidator(this);  
    validatorDepth->setNotation(QDoubleValidator::StandardNotation);  
    validatorDepth->setDecimals(3); // Set the maximum number of decimals to 3  
    captureLengthInput->setValidator(validatorDepth); 
    connect(captureLengthInput, &QLineEdit::editingFinished, this, &MainWindow::onInputDepthValue);

    receptionLayout->addWidget(sampleInputLabel);
    receptionLayout->addWidget(sampleInput);
    receptionLayout->addWidget(captureLengthLabel);
    receptionLayout->addWidget(captureLengthInput);

    xRangeLayout = new QHBoxLayout;
    xRangeLayout->addWidget(xLabel);
    xRangeLayout->addWidget(xRangeInput);
    yRangeLayout = new QHBoxLayout;
    yRangeLayout->addWidget(yLabel);
    yRangeLayout->addWidget(yRangeInput);
    // Configure inputLayout
    yRangeInput->setMaximumSize(200, 100);
    xRangeInput->setMaximumSize(200, 100);
    xLabel->setMaximumSize(50, 100);
    yLabel->setMaximumSize(50, 100);
    xSlider->setMaximumWidth(250);
    ySlider->setMaximumWidth(250);
    inputLayout->addLayout(xRangeLayout,0,0);
    inputLayout->addLayout(yRangeLayout,0,1);
    inputLayout->addWidget(xSlider,1,0);
    inputLayout->addWidget(ySlider,1,1);



    // Configure paramLayout
    paramLayout = new QHBoxLayout();
    paramLayout->addLayout(connectionLayout);
    paramLayout->addLayout(actbuttonLayout);
    paramLayout->addLayout(excitationLayout);
    paramLayout->addLayout(receptionLayout);


    filterLayout = new QGridLayout;
    // Add layouts to mainLayout
    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(paramLayout);
    mainLayout->addLayout(filterLayout);
    centralWidget->setLayout(mainLayout);

    tipsLayout->addWidget(deviceID);
    deviceID->setFixedSize(300, 30); // 设置宽度为 200 像素，高度为 50 像素
    otherTips->setLayout(tipsLayout);
    // Add margins to the main layout
    mainLayout->setContentsMargins(10, 10, 10, 10);
    // Connect signals to slots
    connect(loadDataButton, &QPushButton::clicked, this, &MainWindow::on_loadDataButton_clicked);
    connect(colorButton, &QPushButton::clicked, this, &MainWindow::chooseLineColor);
    connect(autoAdjustButton, &QPushButton::clicked, m_view, &WaveformView::autoAdjustAxes);
    connect(xRangeInput, &QLineEdit::editingFinished, this, &MainWindow::updateAxisRanges);
    connect(yRangeInput, &QLineEdit::editingFinished, this, &MainWindow::updateAxisRanges);
    connect(xSlider, &QSlider::valueChanged, this, &MainWindow::updateXAxisRange);
    connect(ySlider, &QSlider::valueChanged, this, &MainWindow::updateYAxisRange);
    connect(startCollectionButton, &QPushButton::clicked, this, &MainWindow::startDataCollection);
    connect(stopCollectionButton, &QPushButton::clicked, this, &MainWindow::stopDataCollection);
    int time_length = 10; //5ms
    m_controller->vmDevice.channleID = 0;
    m_controller->vmDevice.vmDds.count = 20;
    m_controller->vmDevice.vmDds.cycles = 3;
    m_controller->vmDevice.vmDds.frequency = 10000;
    m_controller->vmDevice.vmDds.intervals = 200;
    m_controller->vmDevice.vmDds.peak = 120;
    m_controller->vmDevice.vmDds.wave = Sine;
    m_controller->vmDevice.vmDos.samplRate = 5000000;
    m_controller->vmDevice.vmDos.depth = time_length * m_controller->vmDevice.vmDos.samplRate/1024000;
    m_controller->vmDevice.vmDos.preSamplDepth = 5;
    connect(m_controller, &WaveformController::deviceIDUpdated, this, &MainWindow::updateDeviceID);
    connect(m_controller, &WaveformController::captureUpdated, this, &MainWindow::toggleButton);
    // Initialize the view with the model
    infoMonitorInital();
    // 创建设备状态线程并启动

}

MainWindow::~MainWindow() {
    delete m_controller;
    delete m_view;
    delete m_model;
}

void MainWindow::updateXAxisRange(int value) {
    double maxValue = m_model->maxXValue();
    double range = maxValue * 2 * value / 100;
    xRangeInput->setText(QString::number(range));
    updateAxisRanges();
}

void MainWindow::infoMonitorInital(){
    m_view->setModel(m_model);
    m_controller->updateDeviceID();
    connect(m_controller->m_vmusbwave, &VmUsbWave::devStateChanged, this, &MainWindow::monitorConnectionStatus);

}


void MainWindow::setConnectionStatus(bool connected) {
    QPixmap icon(connected ? ":/icons/connected.png" : ":/icons/disconnected.png");
    if (icon.isNull()) {
        qDebug() << "Failed to load icon:" << (connected ? ":/icons/connected.png" : ":/icons/disconnected.png");
    }
    statusIconLabel->setPixmap(icon.scaled(ICONSCALE, ICONSCALE, Qt::KeepAspectRatio));
}

void MainWindow::toggleButton() {
    
    actButton->setIcon(QIcon(isRunning ? startButtonIconPath : stopButtonIconPath));
    actStatusLabel->setText(isRunning ?  statusTextStopped:statusTextRunning);
    if (isRunning) {
    m_controller->startDataCollection(); 
    } else {
    m_controller->stopDataCollection();
    }
    isRunning = !isRunning;
}

void MainWindow::updateYAxisRange(int value) {
    double maxValue = m_model->maxYValue();
    double range = maxValue * 2 * value / 100;
    yRangeInput->setText(QString::number(range));
    updateAxisRanges();
}

void MainWindow::on_loadDataButton_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "文件读取", "", tr("Data Files (*.dat);;All Files (*)"));
    if (!fileName.isEmpty()) {
        m_controller->loadData(fileName);
        xSlider->setMaximum(static_cast<int>(m_model->maxXValue() * 2));
        ySlider->setMaximum(static_cast<int>(m_model->maxYValue() * 2));
        xSlider->setValue(100);
        ySlider->setValue(100);
        m_view->autoAdjustAxes();
    }
}

void MainWindow::chooseLineColor() {
    QColor color = QColorDialog::getColor(Qt::black, this, "线形");
    if (color.isValid()) {
        m_view->setLineColor(color);
    }
}

void MainWindow::updateAxisRanges() {
    bool xOk, yOk;
    double xRange = xRangeInput->text().toDouble(&xOk);
    double yRange = yRangeInput->text().toDouble(&yOk);

    if (xOk && yOk) {
        m_view->setAxisRanges(xRange, yRange);
        m_view->update(); // Update the view only after the range is set
    }
}

void MainWindow::startDataCollection() {
    if (!dataCollectionActive) {
        // Initialize the data collection process
        // For example, start a QTimer to periodically collect data
        dataCollectionActive = true;
        // Example of starting data collection (this should be adapted to your actual collection logic)
        qDebug() << "Data collection started";
        // Possibly configure the controller or model to start collecting data
        // m_controller->startDataCollection();
          
    }
}

void MainWindow::stopDataCollection() {
    if (dataCollectionActive) {
        // Stop the data collection process
        dataCollectionActive = false;
        // Example of stopping data collection
        qDebug() << "Data collection stopped";
        // Possibly configure the controller or model to stop collecting data
         m_controller->stopDataCollection();
    }
}

void MainWindow::updateDeviceID(const QString& id) {
    deviceID->setText("设备ID: " + id);
    qDebug() << "Device ID" <<deviceID ;
}

 void MainWindow::monitorConnectionStatus(bool connected){
    setConnectionStatus(connected) ;
 }


void MainWindow::onInputPeakValue() {
    m_controller->vmDevice.vmDds.peak = peakValueInput->text().toDouble()*2000;
    qDebug() << "vPeak: " <<m_controller->vmDevice.vmDds.peak ;
}

void MainWindow::onInputFreValue(){
    m_controller->vmDevice.vmDds.frequency = frequencyInput->text().toInt()*1000;
    qDebug() << "freq: " <<m_controller->vmDevice.vmDds.frequency ;

};
void MainWindow::onInputLoopNumValue(){
    m_controller->vmDevice.vmDds.count = countInput->text().toInt();
    qDebug() << "LoopNum: " <<m_controller->vmDevice.vmDds.count;
};
void MainWindow::onInputCyclesValue(){
    m_controller->vmDevice.vmDds.cycles = periodInput->text().toInt();
    qDebug() << "cycles: " <<m_controller->vmDevice.vmDds.cycles;
    
};

// W_SINE = 0x0001, 
// W_SQUARE = 0x0002, 
// W_RAMP = 0x0004, 
// W_PULSE = 0x0008, 
// W_NOISE = 0x0010, 
// W_DC = 0x0020, 
// W_ARB = 0x0040 
const WaveForm waveStyle[] = {Arb,Sine,Square,Ramp};
void MainWindow::onInputWavestyleValue(){
    m_controller->vmDevice.vmDds.wave = waveStyle[waveformComboBox->currentIndex()];
    qDebug() << "style: " <<m_controller->vmDevice.vmDds.wave;
    
};
void MainWindow::onInputPearidValue(){
    m_controller->vmDevice.vmDds.intervals = intervalInput->text().toInt();
    qDebug() << "Perid: " <<m_controller->vmDevice.vmDds.intervals;
    
};

void MainWindow::onInputSampleValue() {
    m_controller->vmDevice.vmDos.samplRate = (int)sampleInput->text().toDouble()*1e6;
    qDebug() << "sample: " <<m_controller->vmDevice.vmDos.samplRate;
}

void MainWindow::onInputDepthValue() {
    m_controller->vmDevice.vmDos.depth = (int)(captureLengthInput->text().toDouble()* m_controller->vmDevice.vmDos.samplRate/1024000);
    qDebug() << "depth: " <<m_controller->vmDevice.vmDos.depth;
}