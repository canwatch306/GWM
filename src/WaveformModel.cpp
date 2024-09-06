#include "waveformmodel.h"

void WaveformModel::setData(const QVector<double>& x, const QVector<double>& y) {
    m_xData = x;
    m_yData = y;
    emit dataChanged();
}

QVector<double> WaveformModel::xData() const {
    return m_xData;
}

QVector<double> WaveformModel::yData() const {
    return m_yData;
}

double WaveformModel::maxXValue() const {
    if (m_xData.isEmpty()) return 0.0;
    return *std::max_element(m_xData.begin(), m_xData.end());
}

double WaveformModel::maxYValue() const {
    if (m_yData.isEmpty()) return 0.0;
    return *std::max_element(m_yData.begin(), m_yData.end());
}
