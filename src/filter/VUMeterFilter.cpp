#include <cmath>
#include <QDebug>
#include <QtAV/AudioFrame.h>
#include <QtAV/VUMeterFilter.h>

namespace QtAV {
VUMeterFilter::VUMeterFilter(QObject *parent)
    : AudioFilter(parent)
{}

void VUMeterFilter::process(Statistics *statistics, AudioFrame *frame)
{
    if (!frame)
        return;

    const AudioFormat &af = frame->format();
    int step = frame->channelCount();
    const int s = frame->samplesPerChannel();
    float level[2];
    const quint8 *data[2] = {frame->constBits(0), nullptr};

    if (frame->planeCount() > 0) {
        step = 1;
        data[1] = frame->constBits(1);
    } else
        data[1] = data[0] + step * af.sampleSize();

    /** Coletando os dados com base no formato dos frames */
    if (af.isFloat()) {
        float max[2] = {-9999, -9999};
        const float *p0 = (float *) data[0];
        const float *p1 = (float *) data[1];

        /** Se esses valores em data forem nulos, dá zica */
        if (data[0] != nullptr && data[1] != nullptr) {
            for (int i = 0; i < s; i += step) {
                if (data[0] != nullptr)
                    max[0] = qMax(max[0], qAbs(p0[i]));
                if (data[1] != nullptr)
                    max[1] = qMax(max[1], qAbs(p1[i]));
            }
        }

        level[0] = 20 * std::log10(max[0]);
        level[1] = 20 * std::log10(max[1]);

    } else if (!af.isUnsigned()) {
        const int b = af.sampleSize();

        if (b == 2) {
            qint16 max[2] = {-9999, -9999};
            const qint16 *p0 = (qint16 *) data[0];
            const qint16 *p1 = (qint16 *) data[1];

            /** Se esses valores em data forem nulos, dá zica */
            if (data[0] != nullptr && data[1] != nullptr) {
                for (int i = 0; i < s; i += step) {
                    if (data[0] != nullptr)
                        max[0] = qMax(max[0], qAbs(p0[i]));
                    if (data[1] != nullptr)
                        max[1] = qMax(max[1], qAbs(p1[i]));
                }
            }

            level[0] = 20 * std::log10((float) max[0] / INT16_MAX);
            level[1] = 20 * std::log10((float) max[1] / INT16_MAX);
        }
    }

    /** Emitindo vu Esquerdo */
    if (!qFuzzyCompare(level[0], left)) {
        left = level[0];
        Q_EMIT leftLevelChanged(left);
    }

    /** Emitindo vu direito */
    if (!qFuzzyCompare(level[1], right)) {
        right = level[1];
        Q_EMIT rightLevelChanged(right);
    }
}
} // namespace QtAV