#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <map>
#include <memory>

#include "DataPoint.h"
#include "util.h"

#include "SimpleAlgorithm.h"

typedef std::chrono::high_resolution_clock Clock;

std::unique_ptr<Algorithm> selectAlgorithm(std::map<std::string, std::string>& options)
{
    std::unique_ptr<Algorithm> algo;
    std::string algoStr = "simple"; // set default

    if (options.find("-algo") != options.end())
    {
        algoStr = options["-algo"]; // Select algorithm based on option
    }

    if (algoStr == "simple")
    {
        std::cout << "Using \"SimpleAlgorithm\"" << std::endl;
        algo = std::make_unique<SimpleAlgorithm>();
    }
    else
    {
        throw std::runtime_error("Algorithm not found!");
    }

    return algo;
}

std::vector<DataPoint> loadFile(const std::string& filename, bool all, bool withLabel)
{
    std::vector<DataPoint> data;
    std::string entryAmount;
    std::ifstream file(filename);
    if (file.is_open())
    {
        size_t cnt;
        file >> cnt;
        data.reserve(cnt);
        std::cout << "File contains " << cnt << " points" << std::endl;
        //read datapoints
        for (size_t i = 0; i < cnt; i++)
        {
            int x, y, w, h, xL, yL;
            bool isL;
            std::string name;
            file >> x;
            file >> y;
            file >> w;
            file >> h;
            file >> name;
            file >> isL;
            file >> xL;
            file >> yL;

            if (file.fail())
            {
                throw std::runtime_error("Error parsing file: " + filename);
            }

            if (!withLabel)
            {
                isL = xL = yL = 0;
            }

            //onyl add if neccessary
            if (all || isL)
            {
                data.emplace_back(x, y, w, h, name, isL, xL, yL);
            }
        }
    }
    else
    {
        throw std::runtime_error("Error reading file: " + filename);
    }
    return data;
}

void writeFile(const std::string& filename, const std::vector<DataPoint>& data)
{
    std::ofstream outputfile(filename, std::ofstream::trunc);
    if (outputfile.is_open())
    {
        outputfile << data.size() << '\n';
        for (const DataPoint& n : data)
        {
            outputfile << n.xPos << " " << n.yPos << " " << n.width << " " << n.height << " " << n.name << " " << n.isLabeled << " " << n.xLabelTL << " " << n.yLabelTL << '\n';
        }
    }
    else
    {
        throw std::runtime_error("Error writing file: " + filename);
    }
}

int main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        std::cout << "ERROR: Commands missing. Possible commands are:" << std::endl;
        std::cout << "\"-in filename1 -out filename2\" where filename1 is the input file and filename2 is the output file" << std::endl;
        std::cout << "\"-eval filename\" where filename is the file to be evaltuated" << std::endl;
    }
    else
    {
        std::map<std::string, std::string> options;
        //skip first command (is filename)
        for (int i = 1; i < argc; i++)
        {
            std::string cmd = argv[i];
            if (cmd[0] != '-')
            {
                std::cout << "ERROR: Options start at least with one \"-\"" << std::endl;
                return 1;
            }

            if (cmd[1] == '-')
            {
                options.emplace(cmd, std::string());
            }
            else
            {
                if (i + 1 < argc)
                {
                    options.emplace(cmd, argv[i + 1]);
                    i++;
                }
                else
                {
                    std::cout << "ERROR: Missing parameter value for \"" << cmd << "\"!" << std::endl;
                    return 1;
                }
            }
        }

        std::cout << "Running with the following options:" << std::endl;
        for (const auto& o : options)
        {
            if (o.first[1] == '-')
            {
                std::cout << "  " << o.first << std::endl;
            }
            else
            {
                std::cout << "  " << o.first << " = " << o.second << std::endl;
            }
        }

        if (options.find("-eval") != options.end())
        {
            //do evaluation of file
            std::string evalfile = options["-eval"];
            std::vector<DataPoint> evalData = loadFile(evalfile, false, true);

            //check all Labels against all other Labels
            int labelcount = 0;
            for (int i = 0; i < evalData.size(); i++)
            {
                DataPoint entry = evalData[i];
                if (!((entry.xPos == entry.xLabelTL && entry.yPos == entry.yLabelTL)                                    //datapoint is top left
                        || (entry.xPos == entry.xLabelTL + entry.width && entry.yPos == entry.yLabelTL)                     //datapoint is top right
                        || (entry.xPos == entry.xLabelTL && entry.yPos == entry.yLabelTL - entry.height)                    //datapoint is bottom left
                        || (entry.xPos == entry.xLabelTL + entry.width && entry.yPos == entry.yLabelTL - entry.height)))    //datapoint is bottom right
                {
                    std::cout << "ERROR: The Datapoint of Label \"" << evalData[i].name << "\" is not located on any of its corners" << std::endl;
                }

                bool fits = true;
                for (int j = 0; j < evalData.size(); j++)
                {
                    if (i != j)
                    {
                        if (AABBCollision(evalData[j].xLabelTL, evalData[j].yLabelTL, evalData[j].width, evalData[j].height,
                                          evalData[i].xLabelTL, evalData[i].yLabelTL, evalData[i].width, evalData[i].height))
                        {
                            fits = false;
                            std::cout << "ERROR: The Label \"" << evalData[i].name << "\" intersects with Label \"" << evalData[j].name << "\"" << std::endl;
                            //std::cout << evalData[j].name << " " << evalData[j].xLabelTL << " " << evalData[j].yLabelTL << " " << evalData[j].width << " " << evalData[j].height << std::endl;
                            //std::cout << evalData[i].name << " " << evalData[i].xLabelTL << " " << evalData[i].yLabelTL << " " << evalData[i].width << " " << evalData[i].height << std::endl;
                        }
                    }
                }
                if (fits == true)
                {
                    labelcount++;
                }
            }
            //if no label throws an error this is true
            if (labelcount == evalData.size())
            {
                std::cout << "All " << labelcount << " set labels correct!" << std::endl;
            }
        }
        else
        {
            if (options.find("-in") != options.end() && options.find("-out") != options.end())
            {
                std::string infile = options["-in"];
                std::string outfile = options["-out"];
                // Open File
                std::vector<DataPoint> input = loadFile(infile, true, false);

                // Setup algorithm
                std::unique_ptr<Algorithm> algo = selectAlgorithm(options);

                // Start time
                auto t1 = Clock::now();

                // Run algorithm
                algo->run(input);

                // End time
                auto t2 = Clock::now();
                std::chrono::duration<double> elapsed_time = t2 - t1;
                double execution_time = elapsed_time.count();

                // Count set labels
                size_t labelcount = 0;
                for (const DataPoint& p : input)
                {
                    if (p.isLabeled)
                    {
                        labelcount++;
                    }
                }

                std::cout << "Labeled: " << labelcount << '/' << input.size() << " Time: " << std::fixed << std::setprecision(3) << execution_time << "s\n";

                // Write file
                writeFile(outfile, input);
            }
            else
            {
                std::cout << "ERROR: Either \"-in\" or \"-out\" option not given" << std::endl;
                return 1;
            }
        }
    }

    return 0;
}
