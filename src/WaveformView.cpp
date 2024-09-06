#include "WaveformView.h"
#include <QPainter>
#include <QPaintEvent>
#include <algorithm>
#include <QPainterPath>

#define BASE_SIZE 100

WaveformView::WaveformView(QWidget *parent)
    : QWidget(parent), 
      m_model(nullptr), 
      m_lineColor(Qt::black), 
      m_xMin(0), m_xMax(10), 
      m_yMin(0), m_yMax(10)  // 初始化默认坐标轴范围
{}

void WaveformView::setModel(WaveformModel* model) {
    m_model = model;
    update();
}

void WaveformView::setLineColor(const QColor& color) {
    m_lineColor = color;
    update();
}

void WaveformView::setAxisRanges(double xRange, double yRange) {
    m_xMin = 0;
    m_xMax = xRange;
    m_yMin = -yRange;
    m_yMax = yRange;
    update();
}

void WaveformView::autoAdjustAxes() {
    if (!m_model) return;

    const QVector<double> xData = m_model->xData();
    const QVector<double> yData = m_model->yData();

    if (xData.isEmpty() || yData.isEmpty()) return;

    double minX = *std::min_element(xData.begin(), xData.end());
    double maxX = *std::max_element(xData.begin(), xData.end());
    double minY = *std::min_element(yData.begin(), yData.end());
    double maxY = *std::max_element(yData.begin(), yData.end());

    double xBuffer = (maxX - minX) * 0.1;
    double yBuffer = (maxY - minY) * 0.1;

    m_xMin = minX - xBuffer;
    m_xMax = maxX + xBuffer;
    m_yMin = minY - yBuffer;
    m_yMax = maxY + yBuffer;

    update();
}

void WaveformView::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (!m_model) return;
    
    drawGrid(painter);
    drawAxes(painter);
    drawWaveform(painter);
}

void WaveformView::drawGrid(QPainter &painter) {
    painter.setPen(QPen(Qt::lightGray, 1, Qt::DashLine));

    int stepX = (width() - BASE_SIZE*2) / 10; // 网格间隔
    int stepY = (height() - BASE_SIZE*2) / 10;

    for (int i = 1; i <= 10; ++i) {
        int y = height() - BASE_SIZE - i * stepY;
        painter.drawLine(BASE_SIZE, y, width() - BASE_SIZE, y);
    }

    for (int i = 1; i <= 10; ++i) {
        int x = BASE_SIZE + i * stepX;
        painter.drawLine(x, BASE_SIZE, x, height() - BASE_SIZE);
    }
}

void WaveformView::drawAxes(QPainter &painter) {
    painter.setPen(QPen(Qt::black, 2));
    painter.setFont(QFont("Arial", 12, QFont::Bold));

    painter.drawLine(BASE_SIZE, height() - BASE_SIZE, width() - BASE_SIZE, height() - BASE_SIZE);  // X轴
    painter.drawLine(BASE_SIZE, BASE_SIZE, BASE_SIZE, height() - BASE_SIZE);  // Y轴

    drawAxisTicks(painter);  // 调用绘制刻度和标签的方法

 // Adjust text position and font size to avoid clipping
    painter.setFont(QFont("Arial", 10));  // You can reduce the font size if needed
    painter.drawText(width()/2, height() -20, "时间/ms"); // Move the text slightly inward
    painter.save();  // Save the current state of the painter

    painter.translate(40, height() / 2);  // Move the origin to where you want the text to be
    painter.rotate(-90);  // Rotate the text 90 degrees counterclockwise

    painter.setFont(QFont("Arial", 10));  // Font size for Y-axis label
    painter.drawText(0, 0, "电压/V");  // Draw the text at the new origin

    painter.restore();  // Restore the previous state of the painter
}

void WaveformView::drawAxisTicks(QPainter &painter) {
    painter.setPen(QPen(Qt::black, 1));
    painter.setFont(QFont("Arial", 5));

    int numXTicks = 10;
    int numYTicks = 10;

    for (int i = 0; i <= numXTicks; ++i) {
        double xVal = m_xMin + (m_xMax - m_xMin) * i / numXTicks;
        int xPos = BASE_SIZE + i * (width() - BASE_SIZE*2) / numXTicks;
        painter.drawLine(xPos, height() - BASE_SIZE, xPos, height() - BASE_SIZE + 5);
        painter.drawText(xPos - 10, height() - BASE_SIZE + 15, QString::number(xVal, 'f', 1));
    }

    for (int i = 0; i <= numYTicks; ++i) {
        double yVal = m_yMin + (m_yMax - m_yMin) * i / numYTicks;
        int yPos = height() - BASE_SIZE - i * (height() - BASE_SIZE*2) / numYTicks;
        painter.drawLine(BASE_SIZE - 5, yPos, BASE_SIZE, yPos);
        painter.drawText(BASE_SIZE - 30, yPos + 5, QString::number(yVal, 'f', 1));
    }
}

void WaveformView::drawWaveform(QPainter &painter) {
    if (!m_model) return;

    const QVector<double> xData = m_model->xData();
    const QVector<double> yData = m_model->yData();

    if (xData.size() < 2) return;

    QPainterPath path;

    // 计算第一个点的坐标并进行范围检查
    double x0 = (xData[0] - m_xMin) / (m_xMax - m_xMin);
    double y0 = (yData[0] - m_yMin) / (m_yMax - m_yMin);

    if (x0 >= 0 && x0 <= 1 && y0 >= 0 && y0 <= 1) {
        path.moveTo(BASE_SIZE + static_cast<int>(x0 * (width() - BASE_SIZE * 2)),
                    height() - BASE_SIZE - static_cast<int>(y0 * (height() - BASE_SIZE * 2)));
    }

    for (int i = 1; i < xData.size(); ++i) {
        double xNorm = (xData[i] - m_xMin) / (m_xMax - m_xMin);
        double yNorm = (yData[i] - m_yMin) / (m_yMax - m_yMin);

        // 检查点是否在有效范围内
        if (xNorm >= 0 && xNorm <= 1 && yNorm >= 0 && yNorm <= 1) {
            int x = BASE_SIZE + static_cast<int>(xNorm * (width() - BASE_SIZE * 2));
            int y = height() - BASE_SIZE - static_cast<int>(yNorm * (height() - BASE_SIZE * 2));
            path.lineTo(x, y);
        }
    }

    painter.drawPath(path);
}
