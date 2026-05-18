#ifndef __KF__
#define __KF__

#include "../linAlg/Matrix.hpp"
#include "../linAlg/Vector.hpp"

namespace kf
{
    struct Measurement
    {
        unsigned long millis;
        float ax;
        float ay;
        float az;
        float gx;
        float gy;
        float gz;
        float mx;
        float my;
        float mz;
        float temp;
    };

    struct DataPoint
    {
        unsigned long millis;
        float roll;
        float pitch;
        float heading;
    };

    struct KalmanData
    {
        Matrix F;
        Matrix G;
        Matrix R;
        Matrix Q;
        Matrix H;
        Matrix P;
        Matrix K;

        Matrix F_T;
        Matrix H_T;

        Matrix I;

        Vector x_hat;
        Vector u;
        Vector z;
    };

    class KalmanFilter
    {
    private:
        KalmanData &_data;

        int predictEstimate();
        int predictCovariance();
        int updateKalmanGain();
        int updateEstimate();
        int updateCovariance();

    public:
        KalmanFilter(KalmanData &data);

        int kalmanPredict();
        int kalmanUpdate();

        KalmanData &getData();
    };
}

#endif // __KF__