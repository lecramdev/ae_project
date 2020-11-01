#pragma once

#include <vector>

#include "DataPoint.h"

class Algorithm
{
    public:
    virtual ~Algorithm() = default;

    virtual void run(std::vector<DataPoint>& data) = 0;
};
