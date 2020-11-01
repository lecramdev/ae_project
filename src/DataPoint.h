#pragma once

#include <string>

class DataPoint
{
public:
    int xPos, yPos, width, height;
    int xLabelTL, yLabelTL;
    bool isLabeled;
    std::string name;

    DataPoint(): xPos(0), yPos(0), width(0), height(0), name(""), isLabeled(false), xLabelTL(0), yLabelTL(0) {};
    ~DataPoint() = default;
    DataPoint(int x, int y, int w, int h, std::string n): xPos(x), yPos(y), width(w), height(h), name(n), isLabeled(false), xLabelTL(0), yLabelTL(0) {};
    DataPoint(int x, int y, int w, int h, std::string n, bool isL, int xL, int yL): xPos(x), yPos(y), width(w), height(h), name(n), isLabeled(isL), xLabelTL(xL), yLabelTL(yL) {};
};
