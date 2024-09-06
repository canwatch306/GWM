#ifndef WAVEFORMVIEW_H
#define WAVEFORMVIEW_H

#include <QWidget>
#include "WaveformModel.h"

class WaveformView : public QWidget {
    Q_OBJECT

public:
    explicit WaveformView(QWidget *parent = nullptr);
    void setModel(WaveformModel* model);
    void setLineColor(const QColor& color); // 确保这个方法存在
    void setAxisRanges(double xRange, double yRange);  // 设置坐标轴范围
    void autoAdjustAxes();  // 新增：自动调整坐标轴的槽函数
   
    
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    WaveformModel* m_model;
    QColor m_lineColor; // 用于存储折线颜色
    // 新增：用于自适应坐标轴的变量
    double m_xMin;
    double m_xMax;
    double m_yMin;
    double m_yMax;
    void drawGrid(QPainter &painter);
    void drawAxes(QPainter &painter);
    void drawWaveform(QPainter &painter);
    void drawAxisTicks(QPainter &painter);  // 新增的绘制刻度的方法
};

#endif // WAVEFORMVIEW_H
