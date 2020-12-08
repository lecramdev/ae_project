#pragma once
#include <unordered_set>

class ValueEntry
{
public:
    public: 
    int labelValues[4];
    int currentLabel;
    std::unordered_set<int> connectedLabels;
    BoundingBox bbs[4];

    ValueEntry(): labelValues{0,0,0,0}, currentLabel(-1){};
    ~ValueEntry() = default;
    ValueEntry(int l0, int l1,int l2,int l3, int cLabel): labelValues{l0,l1,l2,l3}, currentLabel(cLabel){};
};
