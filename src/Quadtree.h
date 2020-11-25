#pragma once

#include <array>
#include <vector>

#include "DataPoint.h"
#include "BoundingBox.h"

template<uint32_t B>
struct QuadtreeNode
{
    BoundingBox bb; // Bounding box
    uint32_t parent; // index of parent
    uint32_t first_child; // index of first child
    std::array<DataPoint, B> elements;
    uint32_t cnt;
    uint32_t labels;

    QuadtreeNode() : parent(0), first_child(0), cnt(0), labels(0)
    {
    }

    QuadtreeNode(uint32_t parent, const BoundingBox& bb) : bb(bb), parent(parent), first_child(0), cnt(0), labels(0)
    {
    }

    bool has_children()
    {
        return first_child != 0;
    }

    bool is_full()
    {
        return cnt >= B;
    }

    void update_bb(int x, int y)
    {
        if (x < bb.xmin)
        {
            bb.xmin = x;
        }
        else if (x > bb.xmax)
        {
            bb.xmax = x;
        }
        if (y < bb.ymin)
        {
            bb.ymin = y;
        }
        else if (y > bb.ymax)
        {
            bb.ymax = y;
        }
    }

    void update_bb(int xmin, int ymin, int xmax, int ymax)
    {
        if (xmin < bb.xmin)
        {
            bb.xmin = xmin;
        }
        if (xmax > bb.xmax)
        {
            bb.xmax = xmax;
        }
        if (ymin < bb.ymin)
        {
            bb.ymin = ymin;
        }
        if (ymax > bb.ymax)
        {
            bb.ymax = ymax;
        }
    }

    void insert(const DataPoint& val)
    {
        elements[cnt] = val;
        cnt++;
    }
};

template<uint32_t B>
class Quadtree
{
private:
    using Node = QuadtreeNode<B>;
    std::vector<Node> nodes;

    int userNode = 0;
    int userElement = 0;

    Node& get(uint32_t i)
    {
        return nodes[i];
    }

    DataPoint& get(uint32_t i, uint32_t j)
    {
        return nodes[i].elements[j];
    }

    template<bool IGNORE_CURRENT>
    bool collision(int node, const BoundingBox& bb)
    {
        if (node != userNode || IGNORE_CURRENT)
        {
            for (uint32_t i = 0; i < nodes[node].cnt; i++)
            {
                if (bb.collision(nodes[node].elements[i].boundingBox()))
                {
                    return true;
                }
            }
        }
        else
        {
            for (uint32_t i = 0; i < nodes[node].cnt; i++)
            {
                if (i != userElement && bb.collision(nodes[node].elements[i].boundingBox()))
                {
                    return true;
                }
            }
        }

        if (nodes[node].has_children())
        {
            if (nodes[nodes[node].first_child].labels && bb.collision(nodes[nodes[node].first_child].bb) && collision<IGNORE_CURRENT>(nodes[node].first_child, bb))
            {
                return true;
            }
            if (nodes[nodes[node].first_child + 1].labels && bb.collision(nodes[nodes[node].first_child + 1].bb) && collision<IGNORE_CURRENT>(nodes[node].first_child + 1, bb))
            {
                return true;
            }
            if (nodes[nodes[node].first_child + 2].labels && bb.collision(nodes[nodes[node].first_child + 2].bb) && collision<IGNORE_CURRENT>(nodes[node].first_child + 2, bb))
            {
                return true;
            }
            if (nodes[nodes[node].first_child + 3].labels && bb.collision(nodes[nodes[node].first_child + 3].bb) && collision<IGNORE_CURRENT>(nodes[node].first_child + 3, bb))
            {
                return true;
            }
        }

        return false;
    }

public:
    Quadtree(const BoundingBox& bb)
    {
        nodes.emplace_back(0, bb);
    }

    void insert(const DataPoint& val)
    {
        uint32_t node = 0;

        while (true)
        {
            if (nodes[node].has_children())
            {
                // TODO: try direct calculation
                int xsplit = nodes[node].bb.center_x();
                int ysplit = nodes[node].bb.center_y();
                if (val.xPos > xsplit)
                {
                    if (val.yPos > ysplit)
                    {
                        node = nodes[node].first_child;
                    }
                    else
                    {
                        node = nodes[node].first_child + 1;
                    }
                }
                else
                {
                    if (val.yPos > ysplit)
                    {
                        node = nodes[node].first_child + 2;
                    }
                    else
                    {
                        node = nodes[node].first_child + 3;
                    }
                }
            }
            else
            {
                if (nodes[node].is_full())
                {
                    nodes[node].first_child = nodes.size();
                    nodes.emplace_back(node, BoundingBox(nodes[node].bb.center_x(), nodes[node].bb.center_y(), nodes[node].bb.xmax, nodes[node].bb.ymax));
                    nodes.emplace_back(node, BoundingBox(nodes[node].bb.center_x(), nodes[node].bb.ymin, nodes[node].bb.xmax, nodes[node].bb.center_y()));
                    nodes.emplace_back(node, BoundingBox(nodes[node].bb.ymin, nodes[node].bb.center_y(), nodes[node].bb.center_x(), nodes[node].bb.ymax));
                    nodes.emplace_back(node, BoundingBox(nodes[node].bb.ymin, nodes[node].bb.ymin, nodes[node].bb.center_x(), nodes[node].bb.center_y()));
                }
                else
                {
                    nodes[node].insert(val);
                    return;
                }
            }
        }
    }

    void insert(const std::vector<DataPoint>& data)
    {
        for (const DataPoint& val : data)
        {
            insert(val);
        }
    }

    std::vector<DataPoint> to_vector()
    {
        std::vector<DataPoint> vec;
        for (Node& node : nodes)
        {
            for (uint32_t i = 0; i < node.cnt; i++)
            {
                vec.emplace_back(node.elements[i]);
            }
        }
        return vec;
    }

    DataPoint& getCurrent()
    {
        return get(userNode, userElement);
    }

    void setCurrent(LabelPos pos)
    {
        get(userNode, userElement).label = pos;
        BoundingBox bb = get(userNode, userElement).boundingBox();

        int node = userNode;
        while (true)
        {
            Node& n = get(node);
            int labels = 0;
            for (uint32_t i = 0; i < nodes[node].cnt; i++)
            {
                if (get(node, i).isLabeled())
                {
                    bb.update(get(node, i).boundingBox());
                    labels++;
                }
                else
                {
                    bb.update(get(node, i).xPos, get(node, i).yPos);
                }
            }

            if (n.has_children())
            {
                bb.update(get(n.first_child).bb);
                labels += get(n.first_child).labels;
                bb.update(get(n.first_child + 1).bb);
                labels += get(n.first_child + 1).labels;
                bb.update(get(n.first_child + 2).bb);
                labels += get(n.first_child + 2).labels;
                bb.update(get(n.first_child + 3).bb);
                labels += get(n.first_child + 3).labels;
            }

            n.bb = bb;
            n.labels = labels;

            if (node == 0)
            {
                return;
            }

            node = n.parent;
        }
    }

    bool next()
    {
        userElement++;
        if (userElement >= nodes[userNode].cnt)
        {
            userElement = 0;
            userNode++;
            if (userNode >= nodes.size())
            {
                return false;
            }
        }
        return true;
    }

    void reset()
    {
        userNode = 0;
        userElement = 0;
    }

    template<bool IGNORE_CURRENT>
    bool collision(const BoundingBox& bb)
    {
        return nodes[0].labels && collision<IGNORE_CURRENT>(0, bb);
    }
};
