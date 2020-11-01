#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <iomanip>

#include "DataPoint.h"
#include "util.h"

#include "SimpleAlgorithm.h"

typedef std::chrono::high_resolution_clock Clock;

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
    //read commandline arguments
    std::string evalfile;
    std::string infile;
    std::string outfile;

    if (argc <= 1)
    {
        std::cout << "Commands missing. Possible commands are:" << std::endl;
        std::cout << "\"-in filename1 -out filename2\" where filename1 is the input file and filename2 is the output file" << std::endl;
        std::cout << "\"-eval filename\" where filename is the file to be evaltuated" << std::endl;
    }
    else
    {
        //skip first command (is filename)
        for (int i = 1; i < argc; i = i + 2)
        {
            std::string cmd = argv[i];
            if (cmd == "-eval")
            {
                if (i + 1 < argc)
                {
                    evalfile = argv[i + 1];
                }
            }
            else if (cmd == "-in")
            {
                if (i + 1 < argc)
                {
                    infile = argv[i + 1];
                }
            }
            else if (cmd == "-out")
            {
                if (i + 1 < argc)
                {
                    outfile = argv[i + 1];
                }
            }
        }

        if (!evalfile.empty())
        {
            //do evaluation of file
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
            if (infile.empty() || outfile.empty())
            {
                std::cout << "Either -in or -out command has no filename given" << std::endl;
            }
            else
            {
                // Open File
                std::vector<DataPoint> input = loadFile(infile, true, false);

                // Setup algorithm
                SimpleAlgorithm algo;

                // Start time
                auto t1 = Clock::now();

                // Run algorithm
                algo.run(input);

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
        }
    }
}
