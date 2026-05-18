#include "Vector.hpp"

namespace kf
{
    Vector::Vector() : Matrix() {};

    Vector::Vector(size_t n) : Matrix(n, 1) {};

    template <size_t N>
    Vector::Vector(const double array[N]) : Matrix({array}){};

    Vector::Vector(std::initializer_list<double> list)
    {
        _m = list.size();
        _n = 1;
        _data.reserve(_m);

        for (const auto &value : list)
            _data.push_back(value);
    }

    double &Vector::operator()(size_t i) { return Matrix::operator()(i, 0); }

    const double &Vector::operator()(size_t i) const { return Matrix::operator()(i, 0); }

    Vector &Vector::operator=(const Matrix &B)
    {
        if (B.m() != this->m() || B.n() != 1)
            throw std::invalid_argument("Incompatible dimensions for Vector assignment");

        Matrix::operator=(B);
        return *this;
    }

    Vector operator*(const Matrix &A, const Vector &v)
    {
        if (A.n() == 0 || A.m() == 0 || v.m() == 0)
            return Vector();

        if (A.n() != v.m())
            throw std::invalid_argument("Matrix and Vector dimensions must agree for multiplication");

        Vector result(A.m());

        for (size_t i = 0; i < A.m(); i++)
            for (size_t j = 0; j < A.n(); j++)
                result(i) = result(i) + A(i, j) * v(j);

        return result;
    }
}