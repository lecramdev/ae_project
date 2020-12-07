#pragma once

#include "Algorithm.h"
#include "Quadtree.h"

class QuadTreeTestAlgo : public Algorithm
{
public:
    void run(std::vector<DataPoint>& data) override
    {
        BoundingBox bb;
        for (const DataPoint& val : data)
        {
            bb.update(val.xPos, val.yPos);
        }

        Quadtree<128> tree(bb);
        tree.insert(data);

        do
        {
            DataPoint& point = tree.getCurrent();
            point.label = LabelPos::NE;
            if (!tree.collision<false>(point.boundingBox()))
            {
                tree.setCurrent(LabelPos::NE);
            }
            else
            {
                point.label = LabelPos::NONE;
            }
        }
        while (tree.next());

        data = tree.to_vector();
    }
};
