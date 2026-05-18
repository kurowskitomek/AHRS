#ifndef __KFMATRIX__
#define __KFMATRIX__

#include <vector>
#include <ostream>

namespace kf
{

#define TOLERANCE 1e-9

    class Matrix
    {

    protected:
        size_t _m = 0; // rows
        size_t _n = 0; // columns

        std::vector<double> _data;

    public:
        Matrix();
        Matrix(size_t m, size_t n);
        template <size_t M, size_t N>
        Matrix(const double (&array)[M][N]);
        Matrix(std::initializer_list<std::initializer_list<double>> list);

        const double *data() const { return _data.data(); }
        double *data() { return _data.data(); }

        double &operator()(size_t i, size_t j);
        const double &operator()(size_t i, size_t j) const;
        friend Matrix operator+(Matrix A, const Matrix &B);
        friend Matrix operator-(Matrix A, const Matrix &B);
        friend Matrix operator*(double scalar, Matrix A);
        friend Matrix operator*(const Matrix &A, const Matrix &B);
        friend Matrix &operator+=(Matrix &A, const Matrix &B);
        friend Matrix &operator-=(Matrix &A, const Matrix &B);
        friend Matrix &operator*=(Matrix &A, double scalar);
        friend Matrix &operator*=(Matrix &A, const Matrix &B);
        friend bool operator==(const Matrix &A, const Matrix &B);
        friend bool operator!=(const Matrix &A, const Matrix &B);
        Matrix &operator=(const Matrix &B);
        friend std::ostream &operator<<(std::ostream &os, const Matrix &matrix);

        friend double det(Matrix A);
        friend Matrix tp(const Matrix &A);
        friend Matrix inv(Matrix A);

        size_t m() const;
        size_t n() const;
        bool empty() const;
    };

    Matrix Identity(size_t n);
}
#endif //_MATRIX_