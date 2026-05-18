#include <iostream>
#include <vector>
#include <cmath>
#include "KalmanFilter.hpp"

namespace kf
{
    KalmanFilter::KalmanFilter(KalmanData &data) : _data(data)
    {
    }

    KalmanData &KalmanFilter::getData()
    {
        return _data;
    }

    int KalmanFilter::predictEstimate()
    {
        try
        {
            _data.x_hat = _data.F * _data.x_hat + _data.G * _data.u;
        }
        catch (const std::invalid_argument &iaEx)
        {
            std::cerr << iaEx.what() << '\n';
            return -1;
        }
        catch (const std::out_of_range &oorEx)
        {
            std::cerr << oorEx.what() << '\n';
            return -1;
        }

        return 0;
    }

    int KalmanFilter::predictCovariance()
    {
        try
        {
            _data.P = _data.F * _data.P * _data.F_T + _data.Q;
        }
        catch (const std::invalid_argument &iaEx)
        {
            std::cerr << iaEx.what() << '\n';
            return -1;
        }
        catch (const std::out_of_range &oorEx)
        {
            std::cerr << oorEx.what() << '\n';
            return -1;
        }

        return 0;
    }

    int KalmanFilter::updateKalmanGain()
    {
        try
        {
            Matrix PH_Ttemp = _data.P * _data.H_T;
            _data.K = PH_Ttemp * inv(_data.H * PH_Ttemp + _data.R);
        }
        catch (const std::invalid_argument &iaEx)
        {
            std::cerr << iaEx.what() << '\n';
            return -1;
        }
        catch (const std::out_of_range &oorEx)
        {
            std::cerr << oorEx.what() << '\n';
            return -1;
        }

        return 0;
    }

    int KalmanFilter::updateEstimate()
    {
        try
        {
            _data.x_hat += _data.K * (_data.z - _data.H * _data.x_hat);
        }
        catch (const std::invalid_argument &iaEx)
        {
            std::cerr << iaEx.what() << '\n';
            return -1;
        }
        catch (const std::out_of_range &oorEx)
        {
            std::cerr << oorEx.what() << '\n';
            return -1;
        }

        return 0;
    }

    int KalmanFilter::updateCovariance()
    {
        try
        {
            Matrix KH_temp = _data.I - _data.K * _data.H;

            _data.P =
                KH_temp * _data.P * tp(KH_temp) +
                _data.K * _data.R * tp(_data.K);
        }
        catch (const std::invalid_argument &iaEx)
        {
            std::cerr << iaEx.what() << '\n';
            return -1;
        }
        catch (const std::out_of_range &oorEx)
        {
            std::cerr << oorEx.what() << '\n';
            return -1;
        }

        return 0;
    }

    int KalmanFilter::kalmanPredict()
    {
        int resultCode = predictEstimate();

        if (resultCode != 0)
            return -1;

        resultCode = predictCovariance();

        if (resultCode != 0)
            return -1;

        return 0;
    }

    int KalmanFilter::kalmanUpdate()
    {
        int resultCode = updateKalmanGain();

        if (resultCode != 0)
            return -1;

        resultCode = updateEstimate();

        if (resultCode != 0)
            return -1;

        resultCode = updateCovariance();

        if (resultCode != 0)
            return -1;

        return 0;
    }
}