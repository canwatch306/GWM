#ifndef WAVEFORMCONTROLLER_H
#define WAVEFORMCONTROLLER_H

#include "WaveformModel.h"
#include "WaveformView.h"
#include "VmUsbWave.h"  // Include VmUsbWave class
#include <QTimer>
#include <QObject>

enum WaveForm {
    Arb,
    Sine,    // 默认为 1
    Square,  // 默认为 2
    Ramp    // 默认为 3
};

enum CoupleForm {
    AC,    // 默认为 0
    DC // 默认为 1
};

enum DDSOUTMODE {
  CONTINUOUS,
  SWEEP,
  MODE_BURST
};
typedef struct {
    double peak;
    int frequency;
    WaveForm wave;
    DDSOUTMODE mode;
    int count;
    int cycles;
    int intervals;

} EXCEPTIONPARA;

typedef struct {
    int samplRate;
    int depth;
    CoupleForm couple;
    int voltageRange;
    int preSamplDepth;
} RECEPTIONPARA;

typedef struct {
    EXCEPTIONPARA vmDds;
    RECEPTIONPARA vmDos;
    int channleID;
} VMPARAINFO;

typedef struct {
        double version;             // 数据版本，不用显示
        short year;                 // 时间
        short month;
        short day;
        int Mat;                    // 材料
        int Specimen;               // 结构
        int GwMode;                 // 模式
        int WaveSpeed;              // 波速，不用显示
        int Orient;                 // 传播方向
        double GenAmp;              // 信号发生器幅值
        double GenFreq;             // 信号发生器频率
        short GenPeriods;           // 信号发生器周期数 
        double GenInter;            // 信号发生器重复频率
        int SampleFreq;             // 采集频率
        int SampleTime;             // 采集时间
        int SampleTimes;            // 采集次数
        double SampleRange;         // 采集量程
        int SampleResolution;       // 采集灵敏度，不用显示
    } FILEHEAD;  // 这个是老版本定义的结构体，用于保存参数，共占据104字节的空间

class WaveformController : public QObject {
    Q_OBJECT

public:
    explicit WaveformController(WaveformModel* model, WaveformView* view);
    ~WaveformController();
    // 初始化设备
    void initDevice();
    // 清理设备资源
    void cleanupDevice();
    // 加载数据文件
    void loadData(const QString& fileName);
    void updateDeviceID();
    void ddsDeviceInit();
    VMPARAINFO vmDevice;
public slots:
    void startDataCollection();
    void stopDataCollection();
    void processData();

signals:
    void captureUpdated();
    void deviceIDUpdated(const QString& id);
    
public :   
    VmUsbWave* m_vmusbwave;  // Add VmUsbWave pointer

private:
    WaveformModel* m_model;
    WaveformView* m_view;
    double *buffer_ch1;
    FILEHEAD FileHead;  // 这个是为了接收老数据中参数用的
};

#endif // WAVEFORMCONTROLLER_H
