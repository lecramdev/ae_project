#pragma once

#include <algorithm>

#include "Algorithm.h"
#include "LabelQuadtree.h"

#include <gurobi_c++.h>

class ILPAlgorithm : public Algorithm
{
public:
    explicit ILPAlgorithm()
    {
    }

    void run(std::vector<DataPoint>& data) override
    {
        BoundingBox bb;
        for (const DataPoint& val : data)
        {
            bb.update({val.xPos - val.width, val.yPos - val.height, val.xPos + val.width, val.yPos + val.height});
        }

        LabelQuadtree<2048> labeltree(bb);
        for (uint32_t i = 0; i < data.size(); i++)
        {
            labeltree.insert({data[i].boundingBox(LabelPos::NE), i, LabelPos::NE});
            labeltree.insert({data[i].boundingBox(LabelPos::SE), i, LabelPos::SE});
            labeltree.insert({data[i].boundingBox(LabelPos::SW), i, LabelPos::SW});
            labeltree.insert({data[i].boundingBox(LabelPos::NW), i, LabelPos::NW});
        }

        std::cout << "inserted" << std::endl;

        labeltree.update();

        std::cout << "updated" << std::endl;

        auto collision_list = labeltree.collisionList();

        std::cout << "collisions generated" << std::endl;

        try
        {
            // Create an environment
            GRBEnv env = GRBEnv(true);
            env.start();

            // Create an empty model
            GRBModel model = GRBModel(env);

            // Create variables
            std::vector<GRBVar> variables;
            variables.reserve(data.size() * 4);

            for (uint32_t i = 0; i < data.size(); i++)
            {
                variables.emplace_back(model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x_" + std::to_string(i) + "_1"));
                variables.emplace_back(model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x_" + std::to_string(i) + "_2"));
                variables.emplace_back(model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x_" + std::to_string(i) + "_3"));
                variables.emplace_back(model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x_" + std::to_string(i) + "_4"));
            }

            // Set objective
            GRBLinExpr objective;
            for (uint32_t i = 0; i < variables.size(); i++)
            {
                objective += variables[i];
            }
            model.setObjective(objective, GRB_MAXIMIZE);

            // Add constraints
            for (uint32_t i = 0; i < data.size(); i++)
            {
                model.addConstr(variables[i * 4] +
                                variables[i * 4 + 1] +
                                variables[i * 4 + 2] +
                                variables[i * 4 + 3] <= 1);
            }

            for (uint32_t i = 0; i < collision_list.size(); i++)
            {
                for (uint32_t j = 0; j < collision_list[i].size(); j++)
                {
                    if (collision_list[i][j] > i)
                    {
                        model.addConstr(variables[i] + variables[collision_list[i][j]] <= 1);
                    }
                }
            }

            // Optimize model
            model.optimize();

            std::cout << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << std::endl;

            for (uint32_t i = 0; i < data.size(); i++)
            {
                if (variables[i * 4].get(GRB_DoubleAttr_X) == 1.0)
                {
                    data[i].label = LabelPos::NE;
                }
                else if (variables[i * 4 + 1].get(GRB_DoubleAttr_X) == 1.0)
                {
                    data[i].label = LabelPos::SE;
                }
                else if (variables[i * 4 + 2].get(GRB_DoubleAttr_X) == 1.0)
                {
                    data[i].label = LabelPos::SW;
                }
                else if (variables[i * 4 + 3].get(GRB_DoubleAttr_X) == 1.0)
                {
                    data[i].label = LabelPos::NW;
                }
                else
                {
                    data[i].label = LabelPos::NONE;
                }
            }
        }
        catch (GRBException e)
        {
            std::cout << "Error code = " << e.getErrorCode() << std::endl;
            std::cout << e.getMessage() << std::endl;
        }
        catch (...)
        {
            std::cout << "Exception during optimization" << std::endl;
        }
    }
};
