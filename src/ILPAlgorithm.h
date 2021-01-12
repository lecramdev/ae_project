#pragma once

#include <algorithm>

#include "Algorithm.h"
#include "LabelQuadtree.h"

#include <gurobi_c++.h>

class ILPCallback: public GRBCallback
{
public:
    double lastiter;
    double lastnode;
    int numvars;
    GRBVar* vars;
    std::vector<std::vector<uint32_t>>& coll;
    std::vector<uint32_t>& three_pair;
    ILPCallback(int xnumvars, GRBVar* xvars, std::vector<std::vector<uint32_t>>& xcoll, std::vector<uint32_t>& xthree_pair) : coll(xcoll), three_pair(xthree_pair)
    {
        lastiter = lastnode = -GRB_INFINITY;
        numvars = xnumvars;
        vars = xvars;
    }
protected:
    void callback ()
    {
        try
        {
            /*if (where == GRB_CB_MIP)
            {
                // General MIP callback
                double runtime = getDoubleInfo(GRB_CB_RUNTIME);
                double objbst = getDoubleInfo(GRB_CB_MIP_OBJBST);
                double objbnd = getDoubleInfo(GRB_CB_MIP_OBJBND);
                if (fabs(objbst - objbnd) < 1.5 && runtime > 15.0 * 60.0 / 2.0)
                {
                    std::cout << "Stop early - diff < 1.5 after 7.5 minutes" << std::endl;
                    abort();
                }
            }
            else*/ if (where == GRB_CB_MIPNODE)
            {
                // MIP node callback
                //std::cout << "**** New node ****" << std::endl;
                if (getIntInfo(GRB_CB_MIPNODE_STATUS) == GRB_OPTIMAL)
                {
                    double* x = getNodeRel(vars, numvars);

                    /*double max_over = 0.0;
                    uint32_t max_first, max_second, max_third;
                    for (uint32_t i = 0; i < three_pair.size(); i += 3)
                    {
                        uint32_t first = three_pair[i];
                        uint32_t second = three_pair[i + 1];
                        uint32_t third = three_pair[i + 2];
                        if ((x[first] + x[second] + x[third] > max_over))
                        {
                            max_over = x[first] + x[second] + x[third];
                            max_first = first;
                            max_second = second;
                            max_third = third;
                        }
                    }
                    if (max_over > 1.1)
                    {
                        addCut(vars[max_first] + vars[max_second] + vars[max_third] <= 1.0);
                        //std::cout << "add cut " << max_over << '\n';
                    }*/

                    for (int i = 0; i < numvars; i += 4)
                    {
                        bool max_ab = x[i + 0] > x[i + 1];
                        bool max_cd = x[i + 2] > x[i + 3];
                        int order[4];
                        if (x[i + max_ab] > x[i + 2 + max_cd])
                        {
                            order[0] = max_ab;
                            order[1] = 2 + max_cd;
                        }
                        else
                        {
                            order[0] = 2 + max_cd;
                            order[1] = max_ab;
                        }

                        if (x[i + !max_ab] > x[i + 2 + !max_cd])
                        {
                            order[2] = !max_ab;
                            order[3] = 2 + !max_cd;
                        }
                        else
                        {
                            order[2] = 2 + !max_cd;
                            order[3] = !max_ab;
                        }

                        if (x[order[1]] < x[order[2]])
                        {
                            std::swap(order[1], order[2]);
                        }

                        bool use[4];
                        for (int j = 0; j < 4; j++)
                        {
                            use[j] = x[i + order[j]] > 0.1;
                        }

                        for (int j = 0; j < 4; j++)
                        {
                            x[i + j] = 0.0;
                        }

                        for (int j = 0; j < 4; j++)
                        {
                            if (!use[j])
                            {
                                break;
                            }

                            bool isColl = false;
                            for (uint32_t c : coll[i + order[j]])
                            {
                                if (x[c] >= 0.999)
                                {
                                    isColl = true;
                                    break;
                                }
                            }

                            if (!isColl)
                            {
                                x[i + order[j]] = 1.0;
                                break;
                            }
                        }
                    }

                    setSolution(vars, x, numvars);
                    delete[] x;
                }
            }
            /*else if (where == GRB_CB_MIPSOL)
            {
                double* sol = getSolution(vars, numvars);

                for (int i = 0; i < numvars; i += 4)
                {
                    int cnt = 0;
                    for (int j = 0; j < 4; j++)
                    {
                        if (sol[i + j] > 0.5)
                        {
                            cnt++;
                        }
                    }

                    if (cnt > 1)
                    {
                        addLazy(vars[i + 0] + vars[i + 1] + vars[i + 2] + vars[i + 3] <= 1.0);
                    }
                }

                delete[] sol;
            }*/
        }
        catch (GRBException e)
        {
            std::cout << "Error number: " << e.getErrorCode() << ": " << e.getMessage() << std::endl;
        }
        catch (...)
        {
            std::cout << "Error during callback" << std::endl;
        }
    }
};


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

        std::sort(data.begin(), data.end(), [](DataPoint & a, DataPoint & b)
        {
            return (a.width * a.height) < (b.width * b.height);
        });

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
        auto collision_copy = collision_list;

        std::cout << "collisions generated" << std::endl;

        try
        {
            // Create an environment
            GRBEnv env = GRBEnv(true);
            env.start();

            // Create an empty model
            GRBModel model = GRBModel(env);
            //model.set(GRB_IntParam_LazyConstraints, 1);
            model.set(GRB_IntParam_OutputFlag, 0);
            //model.set(GRB_DoubleParam_Heuristics, 0.0);
            model.set(GRB_DoubleParam_TimeLimit, 30.0 * 60.0); // max 30 min.
            /*model.set(GRB_IntParam_Method, 2);
            model.set(GRB_IntParam_NodeMethod, 1);
            model.set(GRB_IntParam_MIPFocus, 2);*/
            //model.set(GRB_IntParam_PreCrush, 1);

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
                auto constr = model.addConstr(variables[i * 4] +
                                              variables[i * 4 + 1] +
                                              variables[i * 4 + 2] +
                                              variables[i * 4 + 3] <= 1);
                //constr.set(GRB_IntAttr_Lazy, 3);
            }

            int cnt = 0;
            std::vector<uint32_t> three_pair;
            /*for (uint32_t first = 0; first < collision_list.size(); first++)
            {
                for (uint32_t j = 0; j < collision_list[first].size(); j++)
                {
                    uint32_t second = collision_list[first][j];
                    if (second != -1 && second > first)
                    {
                        for (uint32_t k = 0; k < collision_list[second].size(); k++)
                        {
                            uint32_t third = collision_list[second][k];
                            if (third != -1 && third > second)
                            {
                                for (uint32_t l = 0; l < collision_list[third].size(); l++)
                                {
                                    uint32_t fourth = collision_list[third][l];
                                    if (fourth == first)
                                    {
                                        cnt++;

                                        three_pair.push_back(first);
                                        three_pair.push_back(second);
                                        three_pair.push_back(third);

                                        // auto constr = model.addConstr(variables[first] + variables[second] + variables[third] <= 1);
                                        // constr.set(GRB_IntAttr_Lazy, -1);
                                        // std::cout << "Cut: " << first << " " << second << " " << third << '\n';

                                        // collision_copy[first][j] = -1;
                                        // collision_copy[second][k] = -1;
                                        // for (uint32_t m = 0; m < collision_list[first].size(); m++)
                                        // {
                                        //     if (collision_list[first][m] == third)
                                        //     {
                                        //         collision_copy[first][m] = -1;
                                        //     }
                                        // }

                                        // auto cut1 = model.addConstr(variables[first] + variables[second] <= 1);
                                        // cut1.set(GRB_IntAttr_Lazy, -1);
                                        // auto cut2 = model.addConstr(variables[first] + variables[third] <= 1);
                                        // cut2.set(GRB_IntAttr_Lazy, -1);
                                        // auto cut3 = model.addConstr(variables[second] + variables[third] <= 1);
                                        // cut3.set(GRB_IntAttr_Lazy, -1);
                                    }
                                }
                            }
                        }
                    }
                }
            }*/
            std::cout << "=======> 3-pairs: " << cnt << '\n';

            for (uint32_t i = 0; i < collision_copy.size(); i++)
            {
                for (uint32_t j = 0; j < collision_copy[i].size(); j++)
                {
                    uint32_t other = collision_copy[i][j];
                    if (other != -1 && other > i)
                    {
                        model.addConstr(variables[i] + variables[other] <= 1);
                    }
                }
            }

            // Set callback
            //ILPCallback callback(variables.size(), variables.data(), collision_list, three_pair);
            //model.setCallback(&callback);

            // Optimize model
            model.optimize();

            std::cout << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << std::endl;

            for (uint32_t i = 0; i < data.size(); i++)
            {
                if (variables[i * 4].get(GRB_DoubleAttr_X) >= 0.5)
                {
                    data[i].label = LabelPos::NE;
                }
                else if (variables[i * 4 + 1].get(GRB_DoubleAttr_X) >= 0.5)
                {
                    data[i].label = LabelPos::SE;
                }
                else if (variables[i * 4 + 2].get(GRB_DoubleAttr_X) >= 0.5)
                {
                    data[i].label = LabelPos::SW;
                }
                else if (variables[i * 4 + 3].get(GRB_DoubleAttr_X) >= 0.5)
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
