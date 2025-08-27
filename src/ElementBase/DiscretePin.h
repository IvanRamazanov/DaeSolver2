#ifndef EBASE_DISCR_PIN_H
#define EBASE_DISCR_PIN_H

#include <cstddef>

class DiscretePin{
    std::size_t size, index;

    public:
        double* getValue();
};

#endif