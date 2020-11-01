#pragma once

#include "Algorithm.h"
#include "util.h"

class SimpleAlgorithm : public Algorithm
{
public:
    void run(std::vector<DataPoint>& data) override
    {
        //our algorithm is stupid and always tries to place the Label at the top right of a datapoint
        //     x/y---------------|
        //      |   TEXT HERE    |
        //      |----------------|
        for (int i = 0; i < data.size(); i++)
        {
            bool fits = true;
            for (int j = 0; j < data.size(); j++)
            {
                if (i != j)
                {
                    if (data[j].isLabeled)
                    {
                        if (AABBCollision(data[j].xPos, data[j].yPos, data[j].width, data[j].height,
                                          data[i].xPos, data[i].yPos, data[i].width, data[i].height))
                        {
                            fits = false;
                        }
                    }
                }
            }

            if (fits == true)
            {
                data[i].isLabeled = true;
                data[i].xLabelTL = data[i].xPos;
                data[i].yLabelTL = data[i].yPos;
            }
            else
            {
                data[i].isLabeled = false;
                data[i].xLabelTL = 0;
                data[i].yLabelTL = 0;
            }
        }
    }
};
