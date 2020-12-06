#pragma once

#include <algorithm>
#include <iostream>
#include <random>

#include "Algorithm.h"
#include "LabelQuadtree.h"

class SimulatedAnnealing : public Algorithm
{
public:
    explicit SimulatedAnnealing()
    {
    }

    void run(std::vector<DataPoint>& data) override
    {
        BoundingBox bb;
        for (const DataPoint& val : data)
        {
            bb.update(val.xPos, val.yPos);
        }

        std::sort(data.begin(), data.end(), [](DataPoint & a, DataPoint & b)
        {
            return (a.width * a.height) < (b.width * b.height);
        });

        LabelQuadtree<128> labeltree(bb);
        // std::vector<Label> labels;
        // labels.reserve(data.size() * 4);
        for (uint32_t i = 0; i < data.size(); i++)
        {
            labeltree.insert({data[i].boundingBox(LabelPos::NE), i, LabelPos::NE});
            labeltree.insert({data[i].boundingBox(LabelPos::SE), i, LabelPos::SE});
            labeltree.insert({data[i].boundingBox(LabelPos::SW), i, LabelPos::SW});
            labeltree.insert({data[i].boundingBox(LabelPos::NW), i, LabelPos::NW});

            // labels.push_back({data[i].boundingBox(LabelPos::NE), i, LabelPos::NE});
            // labels.push_back({data[i].boundingBox(LabelPos::SE), i, LabelPos::SE});
            // labels.push_back({data[i].boundingBox(LabelPos::SW), i, LabelPos::SW});
            // labels.push_back({data[i].boundingBox(LabelPos::NW), i, LabelPos::NW});
        }

        std::cout << "inserted" << std::endl;

        labeltree.update();

        std::cout << "updated" << std::endl;

        auto collision_list = labeltree.collisionList();

        std::cout << "collisions generated" << std::endl;

        // size_t num_coll = 0;
        // size_t comps = 0;
        // for (size_t i = 0; i < labels.size(); i++)
        // {
        //     for (size_t j = i + 1; j < labels.size(); j++)
        //     {
        //         comps++;
        //         if (labels[i].bb.collision(labels[j].bb))
        //         {
        //             num_coll++;
        //         }
        //     }
        // }

        std::vector<bool> labels(data.size() * 4, false);
        std::vector<uint32_t> label_conflicts(data.size() * 4, 0);
        size_t objective = 0;

        std::vector<bool> labels_min;
        std::vector<uint32_t> label_conflicts_min;
        int objective_min;

        std::mt19937 gen(29527987987);
        std::uniform_int_distribution<> rd_point(0, data.size() - 1);
        std::uniform_int_distribution<int8_t> rd_label(0, 4);
        std::uniform_int_distribution<> rd_prob(0.0, 1.0);

        for (size_t i = 0; i < data.size(); i++)
        {
            auto pos = rd_label(gen);
            if (pos)
            {
                size_t idx = i * 4 + pos - 1;
                labels[idx] = true;
                for (uint32_t c : collision_list[idx])
                {
                    if (labels[c])
                    {
                        objective += 2;
                        label_conflicts[idx]++;
                        label_conflicts[c]++;
                    }
                }
            }
            else
            {
                objective += 1;
            }
        }

        labels_min = labels;
        label_conflicts_min = label_conflicts;
        objective_min = objective;

        std::cout << "initial generated" << std::endl;

        double t = 0.5;
        constexpr double delta_t = 0.001;
        for (int it = 0; it < 10000000; it++)
        {
            size_t point = rd_point(gen);
            int8_t pos = rd_label(gen);

            int8_t cur_pos = 0;
            if (labels[point * 4])
            {
                cur_pos = 1;
            }
            else if (labels[point * 4 + 1])
            {
                cur_pos = 2;
            }
            else if (labels[point * 4 + 2])
            {
                cur_pos = 3;
            }
            else if (labels[point * 4 + 3])
            {
                cur_pos = 4;
            }
            else
            {
                cur_pos = 0;
            }

            int delta = 0;
            size_t idx = point * 4 + pos - 1;
            size_t cur_idx = point * 4 + cur_pos - 1;

            if (cur_pos)
            {
                for (uint32_t c : collision_list[cur_idx])
                {
                    if (labels[c])
                    {
                        delta -= 2;
                    }
                }
            }
            else
            {
                delta -= 1;
            }

            if (pos)
            {
                for (uint32_t c : collision_list[idx])
                {
                    if (labels[c])
                    {
                        delta += 2;
                    }
                }
            }
            else
            {
                delta += 1;
            }

            if (delta > 0)
            {
                if (rd_prob(gen) > std::exp(-delta / t))
                {
                    continue;
                }
            }

            if (cur_pos)
            {
                labels[cur_idx] = false;
                label_conflicts[cur_idx] = 0;
                for (uint32_t c : collision_list[cur_idx])
                {
                    if (labels[c])
                    {
                        label_conflicts[c]--;
                    }
                }
            }

            if (pos)
            {
                labels[idx] = true;
                for (uint32_t c : collision_list[idx])
                {
                    if (labels[c])
                    {
                        label_conflicts[idx]++;
                        label_conflicts[c]++;
                    }
                }
            }

            objective += delta;

            if (objective < objective_min)
            {
                labels_min = labels;
                label_conflicts_min = label_conflicts;
                objective_min = objective;
            }
        }

        std::cout << "frozen" << std::endl;

        while (true)
        {
            ssize_t max_idx = -1;
            uint32_t max_conf = 0;
            for (size_t i = 0; i < label_conflicts_min.size(); i++)
            {
                if (label_conflicts_min[i] > max_conf)
                {
                    max_idx = i;
                    max_conf = label_conflicts_min[i];
                }
            }

            if (max_idx >= 0)
            {
                labels_min[max_idx] = false;
                label_conflicts_min[max_idx] = 0;
                for (uint32_t c : collision_list[max_idx])
                {
                    if (labels_min[c])
                    {
                        label_conflicts_min[c]--;
                    }
                }
            }
            else
            {
                break;
            }
        }

        std::cout << "conflicts resolved" << std::endl;

        for (uint32_t i = 0; i < data.size(); i++)
        {
            if (labels_min[i * 4])
            {
                data[i].label = LabelPos::NE;
            }
            else if (labels_min[i * 4 + 1])
            {
                data[i].label = LabelPos::SE;
            }
            else if (labels_min[i * 4 + 2])
            {
                data[i].label = LabelPos::SW;
            }
            else if (labels_min[i * 4 + 3])
            {
                data[i].label = LabelPos::NW;
            }
            else
            {
                data[i].label = LabelPos::NONE;
            }
        }

        // std::cout << "Comps simple: " << comps << std::endl;
        // std::cout << "Collisions: " << std::accumulate(collision_list.begin(), collision_list.end(), 0, [](size_t i, const std::vector<uint32_t>& v)
        // {
        //     return i + v.size();
        // }) << std::endl;
        // std::cout << "Collisions simple: " << num_coll << std::endl;
    }
};
