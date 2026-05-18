#include <stdexcept>
#include <cmath>

#include "Matrix.hpp"

namespace kf
{
    Matrix Identity(size_t n)
    {
        Matrix _I(n, n);

        for (size_t i = 0; i < n; i++)
            _I(i, i) = 1.0;

        return _I;
    }

    size_t Matrix::n() const { return _n; }
    size_t Matrix::m() const { return _m; }
    bool Matrix::empty() const { return _m == 0 || _n == 0; }

    Matrix::Matrix() : _m(0), _n(0), _data() {}

    Matrix::Matrix(size_t m, size_t n) : _m(m), _n(n), _data(m * n, 0.0) {}

    template <size_t M, size_t N>
    Matrix::Matrix(const double (&array)[M][N]) : _m(M), _n(N), _data(M * N)
    {
        for (size_t i = 0; i < M; i++)
            for (size_t j = 0; j < N; j++)
                _data[i * N + j] = array[i][j];
    }

    Matrix::Matrix(std::initializer_list<std::initializer_list<double>> list)
    {
        _m = list.size();
        _n = list.begin()->size();
        _data.reserve(_m * _n);

        for (const auto &row : list)
        {
            if (row.size() != _n)
                throw std::invalid_argument("All rows must have the same number of columns");

            _data.insert(_data.end(), row.begin(), row.end());
        }
    }

    double &Matrix::operator()(size_t i, size_t j)
    {
        if (i >= _m || j >= _n)
            throw std::out_of_range("Matrix index out of range");

        return _data[i * _n + j];
    }

    const double &Matrix::operator()(size_t i, size_t j) const
    {
        if (i >= _m || j >= _n)
            throw std::out_of_range("Matrix index out of range");

        return _data[i * _n + j];
    }

    Matrix operator+(Matrix A, const Matrix &B)
    {
        if (A._n == 0 || A._m == 0)
            return Matrix();

        if (A._m != B._m || A._n != B._n)
            throw std::invalid_argument("Matrix dimensions must agree for addition");

        for (size_t i = 0; i < A._m; i++)
            for (size_t j = 0; j < A._n; j++)
                A(i, j) = A(i, j) + B(i, j);

        return A;
    }

    Matrix operator-(Matrix A, const Matrix &B)
    {
        if (A._n == 0 || A._m == 0)
            return Matrix();

        if (A._m != B._m || A._n != B._n)
            throw std::invalid_argument("Matrix dimensions must agree for subtraction");

        for (size_t i = 0; i < A._m; i++)
            for (size_t j = 0; j < A._n; j++)
                A(i, j) = A(i, j) - B(i, j);

        return A;
    }

    Matrix operator*(double scalar, Matrix A)
    {
        if (A.m() == 0 || A.n() == 0)
            return Matrix();

        const size_t M = A.m();
        const size_t N = A.n();

        double *a = A.data();

        const size_t total = M * N;

        // raw pointer scaling (fast path)
        for (size_t i = 0; i < total; i++)
        {
            a[i] *= scalar;
        }

        return A;
    }

    Matrix operator*(const Matrix &A, const Matrix &B)
    {
        if (A._n == 0 || A._m == 0 || B._n == 0 || B._m == 0)
            return Matrix();

        if (A._n != B._m)
            throw std::invalid_argument("Matrix dimensions must agree for multiplication");

        const size_t Am = A._m;
        const size_t An = A._n;
        const size_t Bn = B._n;

        Matrix C(Am, Bn);

        const double *a = A.data();
        const double *b = B.data();
        double *c = C.data();

        const size_t Csize = Am * Bn;

        // zero output
        for (size_t i = 0; i < Csize; i++)
            c[i] = 0.0;

        for (size_t i = 0; i < Am; i++)
        {
            const size_t a_row = i * An;
            const size_t c_row = i * Bn;

            for (size_t k = 0; k < An; k++)
            {
                const double a_ik = a[a_row + k];
                const size_t b_row = k * Bn;

                size_t j = 0;

                for (; j + 3 < Bn; j += 4)
                {
                    c[c_row + j + 0] += a_ik * b[b_row + j + 0];
                    c[c_row + j + 1] += a_ik * b[b_row + j + 1];
                    c[c_row + j + 2] += a_ik * b[b_row + j + 2];
                    c[c_row + j + 3] += a_ik * b[b_row + j + 3];
                }

                for (; j < Bn; j++)
                {
                    c[c_row + j] += a_ik * b[b_row + j];
                }
            }
        }

        return C;
    }

    Matrix &operator+=(Matrix &A, const Matrix &B)
    {
        A = A + B;
        return A;
    }

    Matrix &operator-=(Matrix &A, const Matrix &B)
    {
        A = A - B;
        return A;
    }

    Matrix &operator*=(Matrix &A, double scalar)
    {
        A = scalar * A;
        return A;
    }

    Matrix &operator*=(Matrix &A, const Matrix &B)
    {
        A = A * B;
        return A;
    }

    bool operator==(const Matrix &A, const Matrix &B)
    {
        if (A._m != B._m || A._n != B._n)
            return false;

        for (size_t i = 0; i < A._m; i++)
            for (size_t j = 0; j < A._n; j++)
                if (A.operator()(i, j) != B(i, j))
                    return false;

        return true;
    }

    bool operator!=(const Matrix &A, const Matrix &B) { return !(A == B); }

    Matrix &Matrix::operator=(const Matrix &B)
    {
        if (this == &B)
            return *this;

        _m = B._m;
        _n = B._n;
        _data = B._data;

        return *this;
    }

    std::ostream &operator<<(std::ostream &os, const Matrix &matrix)
    {
        for (size_t i = 0; i < matrix._m; i++)
        {
            for (size_t j = 0; j < matrix._n; j++)
                os << matrix(i, j) << " ";

            os << std::endl;
        }

        return os;
    }

    double det(Matrix A)
    {
        double num1 = 0;
        double num2 = 0;
        size_t index = 0;
        double total = 1;

        double det = 1.0;

        size_t size = std::min(A._m, A._n);

        std::vector<double> temp(size, 0.0);

        for (size_t i = 0; i < size; i++)
        {
            index = i;

            while (index < size && A(index, i) == 0)
                index++;

            if (index != A._n)
            {
                if (index != i)
                {
                    for (size_t j = 0; j < size; j++)
                        std::swap(A(index, j), A(i, j));

                    det = det * std::pow(-1, index - i);
                }

                for (size_t j = 0; j < size; j++)
                    temp[j] = A(i, j);

                for (size_t j = i + 1; j < size; j++)
                {
                    num1 = temp[i];
                    num2 = A(j, i);

                    for (size_t k = 0; k < size; k++)
                        A(j, k) = (num1 * A(j, k)) - (num2 * temp[k]);

                    total = total * num1;
                }
            }
        }

        for (size_t i = 0; i < size; i++)
            det = det * A(i, i);

        return (det / total);
    }

    Matrix tp(const Matrix &A)
    {
        Matrix A_tran(A._n, A._m);

        for (size_t i = 0; i < A._m; i++)
            for (size_t j = 0; j < A._n; j++)
                A_tran(j, i) = A(i, j);

        return A_tran;
    }

    Matrix inv(Matrix A)
    {
        if (A._m != A._n)
            throw std::invalid_argument("Only square matrices can be inverted");

        double detA = det(A);

        if (std::fabs(detA) < TOLERANCE)
            throw std::invalid_argument("Matrix is singular and cannot be inverted");

        Matrix B = Identity(A._n);

        for (size_t i = 0; i < A._n; ++i)
        {
            double pivot = A(i, i);
            size_t pivotRow = i;
            for (size_t r = i + 1; r < A._n; ++r)
            {
                if (std::fabs(A(r, i)) > std::fabs(pivot))
                {
                    pivot = A(r, i);
                    pivotRow = r;
                }
            }

            if (std::fabs(pivot) < TOLERANCE)
                throw std::runtime_error("Matrix is singular or nearly singular!");

            if (pivotRow != i)
            {
                for (size_t c = 0; c < A._n; ++c)
                {
                    std::swap(A(i, c), A(pivotRow, c));
                    std::swap(B(i, c), B(pivotRow, c));
                }
            }

            double diag = A(i, i);
            for (size_t c = 0; c < A._n; ++c)
            {
                A(i, c) /= diag;
                B(i, c) /= diag;
            }

            for (size_t r = 0; r < A._n; ++r)
            {
                if (r == i)
                    continue;
                double factor = A(r, i);
                for (size_t c = 0; c < A._n; ++c)
                {
                    A(r, c) -= factor * A(i, c);
                    B(r, c) -= factor * B(i, c);
                }
            }
        }

        return B;
    }

}