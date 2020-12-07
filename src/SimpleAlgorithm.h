#pragma once

#include <algorithm>

#include "Algorithm.h"

class SimpleAlgorithm : public Algorithm
{
    bool sort;
public:
    explicit SimpleAlgorithm(bool sort) : sort(sort)
    {
    }

    void run(std::vector<DataPoint>& data) override
    {
        //our algorithm is stupid and always tries to place the Label at the top right of a datapoint
        //     x/y---------------|
        //      |   TEXT HERE    |
        //      |----------------|
        if (sort)
        {
            std::sort(data.begin(), data.end(), [](DataPoint & a, DataPoint & b)
            {
                return (a.width * a.height) < (b.width * b.height);
            });
        }

        for (int i = 0; i < data.size(); i++)
        {
            bool fits = true;
            data[i].label = LabelPos::NE;
            BoundingBox bb = data[i].boundingBox();
            for (int j = 0; j < i; j++)
            {
                if (data[j].isLabeled())
                {
                    if (bb.collision(data[j].boundingBox()))
                    {
                        fits = false;
                        break;
                    }
                }
            }

            if (fits == false)
            {
                data[i].label = LabelPos::NONE;
            }
        }
    }
};
