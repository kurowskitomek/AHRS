#ifndef __KFVECTOR__
#define __KFVECTOR__

#include "Matrix.hpp"

namespace kf
{
    class Vector : public Matrix
    {
    public:
        Vector();
        Vector(size_t n);
        template <size_t N>
        Vector(const double array[N]);
        Vector(std::initializer_list<double> list);

        double &operator()(size_t i);
        const double &operator()(size_t i) const;
        Vector &operator=(const Matrix &B);
        friend Vector operator*(const Matrix &A, const Vector &v);
    };
}
#endif //_VECTOR_