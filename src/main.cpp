#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <iomanip>
//#define DEBUG 

typedef std::chrono::high_resolution_clock Clock;

class DataEntry{
public:
    int xPos, yPos, width, height;
    std::string name;
    bool isLabeled;
    int xLabelTL, yLabelTL; 
    DataEntry():xPos(0), yPos(0), width(0), height(0), name(""), isLabeled(false), xLabelTL(0), yLabelTL(0) {};
    ~DataEntry() {};
    DataEntry(int x, int y, int w, int h, std::string n):xPos(x), yPos(y), width(w), height(h), name(n), isLabeled(false), xLabelTL(0), yLabelTL(0) {}; 
    DataEntry(int x, int y, int w, int h, std::string n, bool isL, int xL, int yL):xPos(x), yPos(y), width(w), height(h), name(n), isLabeled(isL), xLabelTL(xL), yLabelTL(yL) {};
};
bool AABBCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
int main(int argc, char *argv[])
{
    //set path depending on OS
    std::string path = "src\\";

    //read commandline arguments
    std::string eval = "";
    std::string in = "";
    std::string out = "";

    if(argc <= 1){
        std::cout<<"Commands missing. Possible commands are:"<<std::endl;
        std::cout<<"\"-in filename1 -out filename2\" where filename1 is the input file and filename2 is the output file"<<std::endl;
        std::cout<<"\"-eval filename\" where filename is the file to be evaltuated"<<std::endl;
    }else{
        //skip first command (is filename)
        for (int i = 1; i < argc; i = i+2)
        {
            std::string cmd = argv[i];
            if(cmd == "-eval"){
                if(i+1<argc){
                    eval = argv[i+1];
                }
            }else if(cmd == "-in"){
                if(i+1<argc){
                    in = argv[i+1];
                }
            }else if(cmd == "-out"){
                if(i+1<argc){
                    out = argv[i+1];
                }
            }
        }
        if(!eval.empty()){
            //do evaluation of file
            std::vector<DataEntry> evalData;
            std::string entryAmount;
            std::fstream evalfile;
            evalfile.open(path + eval);
            if(evalfile.is_open()){
                std::string fullLine;
                //read first line
                std::getline(evalfile,fullLine);
                entryAmount = fullLine;
                //read datapoints for eval
                while(std::getline(evalfile,fullLine)){
                    std::istringstream iss(fullLine);
                    int x,y,w,h,xL,yL;
                    bool isL;
                    std::string name;
                    std::string tmp;
                    iss>> tmp;
                    x= std::stoi(tmp);
                    iss>> tmp;
                    y= std::stoi(tmp);
                    iss>> tmp;
                    w= std::stoi(tmp);
                    iss>> tmp;
                    h= std::stoi(tmp);
                    iss>>name;
                    iss>>tmp;
                    isL= std::stoi(tmp);
                    iss>>tmp;
                    xL= std::stoi(tmp);
                    iss>>tmp;
                    yL= std::stoi(tmp);

                    //we only evaluate placed Labels
                    if(isL){
                        evalData.push_back(DataEntry(x,y,w,h,name,isL,xL,yL));
                    }

                }
            }else{
                std::cout<<"FILE NOT FOUND: "<<eval<<std::endl;
            }
            evalfile.close();
            
            //check all Labels against all other Labels
            int labelcount = 0;
            for (int i = 0; i < evalData.size(); i++)
            {
                DataEntry entry = evalData[i];
                if(!((entry.xPos == entry.xLabelTL && entry.yPos == entry.yLabelTL)                                     //datapoint is top left
                    ||(entry.xPos == entry.xLabelTL + entry.width && entry.yPos == entry.yLabelTL)                      //datapoint is top right
                    ||(entry.xPos == entry.xLabelTL && entry.yPos == entry.yLabelTL - entry.height)                     //datapoint is bottom left
                    ||(entry.xPos == entry.xLabelTL + entry.width && entry.yPos == entry.yLabelTL - entry.height))){    //datapoint is bottom right
                        std::cout<<"ERROR: The Datapoint of Label \""<< evalData[i].name <<"\" is not located on any of its corners"<<std::endl;
                    }
                
                bool fits = true;
                for (int j = 0; j < evalData.size(); j++)
                {
                    if(i!=j){
                        if(AABBCollision(evalData[j].xLabelTL, evalData[j].yLabelTL-evalData[j].height, evalData[j].width, evalData[j].height, 
                                         evalData[i].xLabelTL, evalData[i].yLabelTL-evalData[i].height, evalData[i].width, evalData[i].height)){
                            fits = false;
                            //std::cout<<"ERROR: The Label \""<< evalData[i].name <<"\" intersects with Label \"" <<evalData[j].name<<"\""<<std::endl;
                            std::cout<<evalData[j].name<<" "<<evalData[j].xLabelTL<<" "<<evalData[j].yLabelTL-evalData[j].height<<" "<<evalData[j].width<<" "<<evalData[j].height<<std::endl;
                            std::cout<<evalData[i].name<<" "<<evalData[i].xLabelTL<<" "<<evalData[i].yLabelTL-evalData[i].height<<" "<<evalData[i].width<<" "<<evalData[i].height<<std::endl;

                        }
                    }
                }
                if(fits == true){
                    labelcount++;
                }
            }
            //if no label throws an error this is true
            if(labelcount == evalData.size()){
                std::cout<< labelcount <<std::endl;
            }


        }else{
            if(in.empty() || out.empty()){
                std::cout<<"Either -in or -out command has no filename given"<<std::endl;
            }else
            {
                //open File
                std::vector<DataEntry> input;
                std::vector<std::string> output;
                std::string entryAmount;
                std::fstream inputfile;
                inputfile.open(path + in);
                if(inputfile.is_open()){
                    std::string fullLine;
                    //read first line
                    std::getline(inputfile,fullLine);
                    entryAmount = fullLine;
                    //read datapoints
                    while(std::getline(inputfile,fullLine)){
                        std::istringstream iss(fullLine);
                        int x,y,w,h;
                        std::string name;
                        std::string tmp;
                        iss>> tmp;
                        x= std::stoi(tmp);
                        iss>> tmp;
                        y= std::stoi(tmp);
                        iss>> tmp;
                        w= std::stoi(tmp);
                        iss>> tmp;
                        h= std::stoi(tmp);
                        iss>>name;
                        input.push_back(DataEntry(x,y,w,h,name));

                        #ifdef DEBUG
                        std::string s;
                        std::stringstream ss(s);
                        ss << x <<" "<<y <<" "<<w<<" "<<h<<" "<<name<<" "<< "0 0 0";
                        s = ss.str();
                        output.push_back(s);
                        #endif
                    }
                }else{
                    std::cout<<"FILE NOT FOUND: "<<in<<std::endl;
                }
                inputfile.close();

                //our algorithm is stupid and always tries to place the Label at the top right of a datapoint
                //      |----------------|
                //      |   TEXT HERE    |
                //     x/y---------------|
                auto t1 = Clock::now();

                int labelcount = 0;
                for (int i = 0; i < input.size(); i++)
                {
                    bool fits = true;
                    for (int j = 0; j < input.size(); j++)
                    {
                        if(i!=j){
                            if(input[j].isLabeled){
                                if(AABBCollision(input[j].xPos, input[j].yPos, input[j].width, input[j].height, 
                                                 input[i].xPos, input[i].yPos, input[i].width, input[i].height)){
                                    fits = false;
                                }
                            }
                        }
                    }
                    if(fits == true){
                        labelcount++;
                        input[i].isLabeled = true;
                        input[i].xLabelTL = input[i].xPos;
                        input[i].yLabelTL = input[i].yPos + input[i].height;
                    }else
                    {
                        input[i].isLabeled = false;
                        input[i].xLabelTL = 0;
                        input[i].yLabelTL = 0;
                    }
                    
                }

                auto t2 = Clock::now();
                std::chrono::duration<double> elapsed_time = t2-t1;
                double execution_time = elapsed_time.count();
                std::cout<<labelcount<<'\t'<<std::fixed << std::setprecision(3)<<execution_time<<std::endl;

                //generate output file
                std::fstream outputfile;
                outputfile.open(path + out, std::ofstream::out | std::ofstream::trunc);
                if(outputfile.is_open()){
                    outputfile<<entryAmount<<'\n';
                    for(DataEntry n : input) {
                        outputfile<<n.xPos<<" "<<n.yPos<<" "<<n.width<<" "<<n.height<<" "<<n.name<<" "<<n.isLabeled<<" "<<n.xLabelTL<<" "<<n.yLabelTL<<'\n';
                        
                        #ifdef DEBUG
                            std::cout<<n.xPos<<" "<<n.yPos<<" "<<n.width<<" "<<n.height<<" "<<n.name<<" "<<n.isLabeled<<" "<<n.xLabelTL<<" "<<n.yLabelTL<<std::endl;
                        #endif
                    }

                }else{
                    std::cout<<"FILE NOT FOUND: "<<out<<std::endl;
                }
                outputfile.close();
            }
        }
    }

}
//x/y is the bottom left coordinate of a box
bool AABBCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2){
    if(x1 < (x2+w2) && (x1+w1) > x2 && y1 < (y2+h2) && (y1+h1) > y2)
    {
        return true;
    }
    else
    {
        return false;
    }
}

