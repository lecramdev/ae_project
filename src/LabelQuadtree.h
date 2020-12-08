#pragma once

#include <array>
#include <vector>
#include <deque>
#include <unordered_set>

#include "DataPoint.h"
#include "BoundingBox.h"

struct Label
{
    BoundingBox bb;
    uint32_t point;
    LabelPos pos;

    BoundingBox& boundingBox()
    {
        return bb;
    }

    int xPos() const
    {
        return bb.center_x();
    }

    int yPos() const
    {
        return bb.center_y();
    }
};

namespace std
{
template<> struct hash<pair<uint32_t, uint32_t>>
{
    std::size_t operator()(const pair<uint32_t, uint32_t>& p) const
    {
        std::size_t h1 = std::hash<uint32_t> {}(p.first);
        std::size_t h2 = std::hash<uint32_t> {}(p.second);
        return h1 ^ (h2 << 1); // or use boost::hash_combine
    }
};
}

template<uint32_t B>
struct LabelQuadtreeNode
{
    BoundingBox bb; // Bounding box
    //uint32_t parent; // index of parent
    uint32_t first_child; // index of first child
    std::array<Label, B> elements;
    uint32_t cnt;

    LabelQuadtreeNode() : first_child(0), cnt(0)
    {
    }

    explicit LabelQuadtreeNode(const BoundingBox& bb) : bb(bb), first_child(0), cnt(0)
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

    void insert(const Label& val)
    {
        elements[cnt] = val;
        cnt++;
    }
};

template<uint32_t B>
class LabelQuadtree
{
private:
    using Node = LabelQuadtreeNode<B>;
    std::vector<Node> nodes;

    size_t size = 0;

    int userNode = 0;
    int userElement = 0;

    Node& get(uint32_t i)
    {
        return nodes[i];
    }

    Label& get(uint32_t i, uint32_t j)
    {
        return nodes[i].elements[j];
    }

public:
    LabelQuadtree(const BoundingBox& bb)
    {
        nodes.emplace_back(bb);
    }

    void insert(const Label& val)
    {
        uint32_t node = 0;

        while (true)
        {
            if (get(node).has_children())
            {
                // TODO: try direct calculation
                int xsplit = get(node).bb.center_x();
                int ysplit = get(node).bb.center_y();
                if (val.xPos() > xsplit)
                {
                    if (val.yPos() > ysplit)
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
                    if (val.yPos() > ysplit)
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
                    nodes.emplace_back(BoundingBox(get(node).bb.center_x(), get(node).bb.center_y(), get(node).bb.xmax, get(node).bb.ymax));
                    nodes.emplace_back(BoundingBox(get(node).bb.center_x(), get(node).bb.ymin, get(node).bb.xmax, get(node).bb.center_y()));
                    nodes.emplace_back(BoundingBox(get(node).bb.ymin, get(node).bb.center_y(), get(node).bb.center_x(), get(node).bb.ymax));
                    nodes.emplace_back(BoundingBox(get(node).bb.ymin, get(node).bb.ymin, get(node).bb.center_x(), get(node).bb.center_y()));
                }
                else
                {
                    get(node).insert(val);
                    size++;
                    return;
                }
            }
        }
    }

    void insert(const std::vector<Label>& data)
    {
        for (const Label& val : data)
        {
            insert(val);
        }
    }

    void update()
    {
        for (ssize_t node = nodes.size() - 1; node >= 0; node--)
        {
            get(node).bb = BoundingBox(0, 0, 0, 0);

            for (uint32_t i = 0; i < get(node).cnt; i++)
            {
                get(node).bb.update(get(node, i).boundingBox());
            }

            if (get(node).has_children())
            {
                get(node).bb.update(get(get(node).first_child).bb);
                get(node).bb.update(get(get(node).first_child + 1).bb);
                get(node).bb.update(get(get(node).first_child + 2).bb);
                get(node).bb.update(get(get(node).first_child + 3).bb);
            }
        }
    }

    std::vector<Label> to_vector()
    {
        std::vector<Label> vec;
        for (Node& node : nodes)
        {
            for (uint32_t i = 0; i < node.cnt; i++)
            {
                vec.emplace_back(node.elements[i]);
            }
        }
        return vec;
    }

    Label& getCurrent()
    {
        return get(userNode, userElement);
    }

    bool next()
    {
        userElement++;
        while (userElement >= get(userNode).cnt)
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

    std::vector<std::vector<uint32_t>> collisionList()
    {
        std::vector<std::vector<uint32_t>> list(size);

        std::vector<uint32_t> queue;
        queue.resize(nodes.size());
        uint32_t start = 0;
        uint32_t end = 0;

        // size_t max_size = 0;
        // size_t next_print = 2000;

        reset();
        do
        {
            // std::deque<int> queue;
            // queue.push_back(0);
            start = 0;
            end = 1;
            queue[start] = 0;

            while (start != end)
                // while (!queue.empty())
            {
                // int node = queue.front();
                // queue.pop_front();
                int node = queue[start];
                start++;

                if (node > userNode)
                {
                    for (uint32_t i = 0; i < get(node).cnt; i++)
                    {
                        if (getCurrent().bb.collision(get(node, i).boundingBox()))
                        {
                            // list.emplace_back(getCurrent().point << 2 + (int8_t)getCurrent().pos, get(node, i).point << 2 + (int8_t)get(node, i).pos);
                            list[(getCurrent().point << 2) + (int8_t)getCurrent().pos].emplace_back((get(node, i).point << 2) + (int8_t)get(node, i).pos);
                            // max_size = std::max(list[(getCurrent().point << 2) + (int8_t)getCurrent().pos].size(), max_size);
                            list[(get(node, i).point << 2) + (int8_t)get(node, i).pos].emplace_back((getCurrent().point << 2) + (int8_t)getCurrent().pos);
                            // max_size = std::max(list[(get(node, i).point << 2) + (int8_t)get(node, i).pos].size(), max_size);
                        }
                    }
                }
                else if (node == userNode)
                {
                    for (uint32_t i = userElement + 1; i < get(node).cnt; i++)
                    {
                        if (getCurrent().bb.collision(get(node, i).boundingBox()))
                        {
                            // list.emplace_back(getCurrent().point << 2 + (int8_t)getCurrent().pos, get(node, i).point << 2 + (int8_t)get(node, i).pos);
                            list[(getCurrent().point << 2) + (int8_t)getCurrent().pos].emplace_back((get(node, i).point << 2) + (int8_t)get(node, i).pos);
                            // max_size = std::max(list[(getCurrent().point << 2) + (int8_t)getCurrent().pos].size(), max_size);
                            list[(get(node, i).point << 2) + (int8_t)get(node, i).pos].emplace_back((getCurrent().point << 2) + (int8_t)getCurrent().pos);
                            // max_size = std::max(list[(get(node, i).point << 2) + (int8_t)get(node, i).pos].size(), max_size);
                        }
                    }
                }

                if (get(node).has_children())
                {
                    int i = 0;
                    if (getCurrent().bb.collision(get(get(node).first_child).bb))
                    {
                        // queue.emplace_back(get(node).first_child);
                        queue[end] = get(node).first_child;
                        end++;
                    }
                    if (getCurrent().bb.collision(get(get(node).first_child + 1).bb))
                    {
                        // queue.emplace_back(get(node).first_child + 1);
                        queue[end] = get(node).first_child + 1;
                        end++;
                    }
                    if (getCurrent().bb.collision(get(get(node).first_child + 2).bb))
                    {
                        // queue.emplace_back(get(node).first_child + 2);
                        queue[end] = get(node).first_child + 2;
                        end++;
                    }
                    if (getCurrent().bb.collision(get(get(node).first_child + 3).bb))
                    {
                        // queue.emplace_back(get(node).first_child + 3);
                        queue[end] = get(node).first_child + 3;
                        end++;
                    }
                }
            }

            // max_size++;
            // if (max_size > next_print)
            // {
            //     std::cout << "Max: " << std::accumulate(list.begin(), list.end(), 0UL, [](size_t i, const std::vector<uint32_t>& v)
            //     {
            //         return i + v.size();
            //     }) << std::endl;
            //     next_print += 2000;
            // }
        }
        while (next());

        // std::cout << "Max: " << max_size << std::endl;

        return list;
    }
};
