#ifndef WAVEFORMMODEL_H
#define WAVEFORMMODEL_H

#include <QObject>
#include <QVector>
#include <algorithm>  // For std::max_element

class WaveformModel : public QObject {
    Q_OBJECT

public:
    void setData(const QVector<double>& x, const QVector<double>& y);
    QVector<double> xData() const;
    QVector<double> yData() const;
    double maxXValue() const;
    double maxYValue() const;

signals:
    void dataChanged();

private:
    QVector<double> m_xData;
    QVector<double> m_yData;
};

#endif // WAVEFORMMODEL_H
