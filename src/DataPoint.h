#pragma once

#include <string>

#include "BoundingBox.h"

enum class LabelPos : int8_t
{
    NONE,
    NE,
    SE,
    SW,
    NW
};

class DataPoint
{
public:
    int xPos, yPos, width, height;
    LabelPos label;
    std::string name;

    DataPoint(): xPos(0), yPos(0), width(0), height(0), name(""), label(LabelPos::NONE) {};
    ~DataPoint() = default;
    DataPoint(int x, int y, int w, int h, std::string n): xPos(x), yPos(y), width(w), height(h), name(n), label(LabelPos::NONE) {};
    DataPoint(int x, int y, int w, int h, std::string n, LabelPos l): xPos(x), yPos(y), width(w), height(h), name(n), label(l) {};

    bool isLabeled() const
    {
        return label != LabelPos::NONE;
    }

    int xLabelTL() const
    {
        switch (label)
        {
        case LabelPos::NE:
        case LabelPos::SE:
            return xPos;

        case LabelPos::SW:
        case LabelPos::NW:
            return xPos - width;

        default:
            return 0;
        }
    }

    int yLabelTL() const
    {
        switch (label)
        {
        case LabelPos::NE:
        case LabelPos::NW:
            return yPos + height;

        case LabelPos::SE:
        case LabelPos::SW:
            return yPos;

        default:
            return 0;
        }
    }

    BoundingBox boundingBox() const
    {
        switch (label)
        {
        case LabelPos::NE:
            return BoundingBox(xPos, yPos, xPos + width, yPos + height);
        case LabelPos::SE:
            return BoundingBox(xPos, yPos - height, xPos + width, yPos);
        case LabelPos::SW:
            return BoundingBox(xPos - width, yPos - height, xPos, yPos);
        case LabelPos::NW:
            return BoundingBox(xPos - width, yPos, xPos, yPos + height);

        default:
            return BoundingBox(0, 0, 0, 0);
        }
    }
};
