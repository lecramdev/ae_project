#pragma once

#include <array>
#include <vector>
#include <deque>

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
            if (get(node).has_children())
            {
                // TODO: try direct calculation
                int xsplit = get(node).bb.center_x();
                int ysplit = get(node).bb.center_y();
                if (val.xPos > xsplit)
                {
                    if (val.yPos > ysplit)
                    {
                        node = get(node).first_child;
                    }
                    else
                    {
                        node = get(node).first_child + 1;
                    }
                }
                else
                {
                    if (val.yPos > ysplit)
                    {
                        node = get(node).first_child + 2;
                    }
                    else
                    {
                        node = get(node).first_child + 3;
                    }
                }
            }
            else
            {
                if (get(node).is_full())
                {
                    get(node).first_child = nodes.size();
                    nodes.emplace_back(node, BoundingBox(get(node).bb.center_x(), get(node).bb.center_y(), get(node).bb.xmax, get(node).bb.ymax));
                    nodes.emplace_back(node, BoundingBox(get(node).bb.center_x(), get(node).bb.ymin, get(node).bb.xmax, get(node).bb.center_y()));
                    nodes.emplace_back(node, BoundingBox(get(node).bb.ymin, get(node).bb.center_y(), get(node).bb.center_x(), get(node).bb.ymax));
                    nodes.emplace_back(node, BoundingBox(get(node).bb.ymin, get(node).bb.ymin, get(node).bb.center_x(), get(node).bb.center_y()));
                }
                else
                {
                    get(node).insert(val);
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
            int labels = 0;
            for (uint32_t i = 0; i < get(node).cnt; i++)
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

            if (get(node).has_children())
            {
                bb.update(get(get(node).first_child).bb);
                labels += get(get(node).first_child).labels;
                bb.update(get(get(node).first_child + 1).bb);
                labels += get(get(node).first_child + 1).labels;
                bb.update(get(get(node).first_child + 2).bb);
                labels += get(get(node).first_child + 2).labels;
                bb.update(get(get(node).first_child + 3).bb);
                labels += get(get(node).first_child + 3).labels;
            }

            get(node).bb = bb;
            get(node).labels = labels;

            if (node == 0)
            {
                return;
            }

            node = get(node).parent;
        }
    }

    bool next()
    {
        userElement++;
        if (userElement >= get(userNode).cnt)
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
        if (nodes[0].labels)
        {
            std::deque<int> stack;
            stack.push_back(0);

            while (!stack.empty())
            {
                int node = stack.front();
                stack.pop_front();

                if (node != userNode || IGNORE_CURRENT)
                {
                    for (uint32_t i = 0; i < get(node).cnt; i++)
                    {
                        if (get(node, i).isLabeled() && bb.collision(get(node, i).boundingBox()))
                        {
                            return true;
                        }
                    }
                }
                else
                {
                    for (uint32_t i = 0; i < get(node).cnt; i++)
                    {
                        if (i != userElement && get(node, i).isLabeled() && bb.collision(get(node, i).boundingBox()))
                        {
                            return true;
                        }
                    }
                }

                if (get(node).has_children())
                {
                    if (get(get(node).first_child).labels && bb.collision(get(get(node).first_child).bb))
                    {
                        stack.push_back(get(node).first_child);
                    }
                    if (get(get(node).first_child + 1).labels && bb.collision(get(get(node).first_child + 1).bb))
                    {
                        stack.push_back(get(node).first_child + 1);
                    }
                    if (get(get(node).first_child + 2).labels && bb.collision(get(get(node).first_child + 2).bb))
                    {
                        stack.push_back(get(node).first_child + 2);
                    }
                    if (get(get(node).first_child + 3).labels && bb.collision(get(get(node).first_child + 3).bb))
                    {
                        stack.push_back(get(node).first_child + 3);
                    }
                }
            }
        }
        return false;
    }
};
