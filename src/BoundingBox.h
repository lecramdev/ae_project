#pragma once

#include <limits>

struct BoundingBox
{
    int xmin, ymin, xmax, ymax;

    BoundingBox() :
        xmin(std::numeric_limits<int>::max()), ymin(std::numeric_limits<int>::max()),
        xmax(std::numeric_limits<int>::min()), ymax(std::numeric_limits<int>::min())
    {
    }

    BoundingBox(int xmin, int ymin, int xmax, int ymax) : xmin(xmin), ymin(ymin), xmax(xmax), ymax(ymax)
    {
    }

    bool collision(const BoundingBox& bb) const
    {
        return xmin < bb.xmax && xmax > bb.xmin && ymin < bb.ymax && ymax > bb.ymin;
    }

    int center_x() const
    {
        return (xmin + xmax) / 2;
    }

    int center_y() const
    {
        return (ymin + ymax) / 2;
    }

    void update(int x, int y)
    {
        if (x < xmin)
        {
            xmin = x;
        }
        else if (x > xmax)
        {
            xmax = x;
        }
        if (y < ymin)
        {
            ymin = y;
        }
        else if (y > ymax)
        {
            ymax = y;
        }
    }

    void update(const BoundingBox& bb)
    {
        if (bb.xmin < xmin)
        {
            xmin = bb.xmin;
        }
        if (bb.xmax > xmax)
        {
            xmax = bb.xmax;
        }
        if (bb.ymin < ymin)
        {
            ymin = bb.ymin;
        }
        if (bb.ymax > ymax)
        {
            ymax = bb.ymax;
        }
    }
};
