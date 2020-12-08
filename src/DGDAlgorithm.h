#pragma once

#include "Algorithm.h"
#include "DGDDatapoint.h"
//#include "util.h"
#include <time.h>
#include <vector>
#include <iostream>
#include "DGDValueEntry.h"
/*
A Discrete Gradient Descent algorithm. 
1: place all labels randomly
2: Until there is no improvement left:
    a) for each feature try moving the label to the other positions
    b) for each repositioning, calculate the improvement
    c) implement the repositioning that results in improvement
In practice table of costs associated with each repositioning precomputed. Then after repositioning only touched label positions recomputed

Based on this paper:
https://dash.harvard.edu/bitstream/handle/1/2032678/AnEmpiricalStudy.pdf
*/
class DGDAlgorithm : public Algorithm
{
public:
    int configurationValue = 0;
    std::vector<DGDDataPoint> DGDdata;
    void run(std::vector<DataPoint>& data) override
    {
        DGDdata.reserve(data.size());
        for(int i = 0; i<data.size();i++){
            DGDdata.emplace_back(data[i].xPos, data[i].yPos, data[i].width, data[i].height, data[i].name);
        }

        //Step 1: All Labels are assigned a random position
        //|--------|--------|
        //|    2   |    3   |
        //|--------X--------|
        //|    1   |    0   |
        //|--------|--------|
        srand (time(NULL));
        std::vector<ValueEntry> values;
        values.reserve(DGDdata.size());
        for (int i = 0; i < DGDdata.size(); i++)
        {
            values.emplace_back(0,0,0,0,-1);
            int randpos = rand() % 4;
            DGDdata[i].isLabeled = true;
            applyLabelToPos(values, DGDdata, i, randpos);
            
        }
        std::cout<<"total: "<< configurationValue <<std::endl;
        //Step 2 set up all label position values:
        setupLabelConnections(values, DGDdata);
        std::cout<<"total: "<< configurationValue <<std::endl;
        
        //step 3: until no further improvement is possible:
        int countdown = 2;
        int lastValue = 1;
        while(countdown >= 0){
            
            //a) for each label consider the best position and move it there
            for (int i = 0; i < values.size(); i++)
            {
                int oldPos = values[i].currentLabel;
                int oldxTL = DGDdata[i].xLabelTL;
                int oldyTL = DGDdata[i].yLabelTL;
                int newPos = getSmallestIndex(values[i]);
                //std::cout<<oldPos<<" vs. "<< newPos<<std::endl;
                applyLabelToPos(values, DGDdata, i, newPos);
                                
                //update total configurationValue
                configurationValue -= values[i].labelValues[oldPos];
                configurationValue += values[i].labelValues[newPos];
                //update the values of the surrounding labels
                updateLocalLabels(values, DGDdata, i, oldxTL, oldyTL);
                
                
            }
            if(lastValue == configurationValue){
                countdown--;
            }else if(lastValue < configurationValue + 2 && lastValue > configurationValue - 2){
                countdown --;
            }else{
                countdown == 2;
            }
            //std::cout<<configurationValue<<std::endl;
            lastValue = configurationValue;
        }

        //delete all overlapping labels
        for (int i = 0; i < values.size(); i++)
        {
            if(values[i].labelValues[values[i].currentLabel] > 0){
                configurationValue -= values[i].labelValues[values[i].currentLabel];
                deleteLabel(values, DGDdata, i);
                DGDdata[i].isLabeled = false;
                DGDdata[i].xLabelTL = 0;
                DGDdata[i].yLabelTL = 0;
                data[i].label = LabelPos::NONE;
            }else{
            switch (values[i].currentLabel)
                {
                case 0:
                    data[i].label = LabelPos::SE;
                    break;
                case 1:
                    data[i].label = LabelPos::SW;
                    break;
                case 2:
                    data[i].label = LabelPos::NW;
                    break;
                case 3:
                    data[i].label = LabelPos::NE;
                    break;
                
                default:
                    break;
                }
            }
            //std::cout<< "Point " << i <<", with label "<< values[i].currentLabel <<" has values: L0 = " << values[i].labelValues[0]<<", L1 = "<<values[i].labelValues[1]<<", L2 = " <<values[i].labelValues[2]<<", L3 = "<<values[i].labelValues[3]<<std::endl;

            
        }

        
        
        
    }


    /*Updates the values in the DGDdata vector depending on the given position */
    void applyLabelToPos(std::vector<ValueEntry>& values, std::vector<DGDDataPoint>& DGDdata, int i, int pos){
        switch(pos) {
            case 0:
                DGDdata[i].xLabelTL = DGDdata[i].xPos;
                DGDdata[i].yLabelTL = DGDdata[i].yPos;
                values[i].currentLabel = 0; 
                break;
            case 1:
                DGDdata[i].xLabelTL = DGDdata[i].xPos - DGDdata[i].width;
                DGDdata[i].yLabelTL = DGDdata[i].yPos;
                values[i].currentLabel = 1; 
                break;
            case 2:
                DGDdata[i].xLabelTL = DGDdata[i].xPos - DGDdata[i].width;
                DGDdata[i].yLabelTL = DGDdata[i].yPos + DGDdata[i].height;
                values[i].currentLabel = 2; 
                break;
            case 3:
                DGDdata[i].xLabelTL = DGDdata[i].xPos;
                DGDdata[i].yLabelTL = DGDdata[i].yPos + DGDdata[i].height;
                values[i].currentLabel = 3; 
                break;
            default:
                break;
        }
    }

    /*Updates all Label values in the Value array after the Label in value index was deleted*/
    void deleteLabel(std::vector<ValueEntry>& values, std::vector<DGDDataPoint>& DGDdata, int valueIndex){
        //currentVE is the just updated point with its label
        ValueEntry currentVE = values[valueIndex];
        //For all neighboring labels, update their values (using their connected labels)
        for (const auto& neighbourDirect: currentVE.connectedLabels) {
            ValueEntry connectedVE = values[neighbourDirect];
            configurationValue -= connectedVE.labelValues[currentVE.currentLabel];
            int i = neighbourDirect;
            for(int lno = 0; lno <4; lno++){
                switch (lno)
                {
                    //check if there was a collision with the deleted label (remove 1 value) 
                case 0:
                    if (AABBCollision(DGDdata[valueIndex].xLabelTL, DGDdata[valueIndex].yLabelTL, DGDdata[valueIndex].width, DGDdata[valueIndex].height,
                                    DGDdata[i].xPos, DGDdata[i].yPos, DGDdata[i].width, DGDdata[i].height)){
                        values[neighbourDirect].labelValues[lno]--;
                    }
                    break;
                case 1:
                    if (AABBCollision(DGDdata[valueIndex].xLabelTL, DGDdata[valueIndex].yLabelTL, DGDdata[valueIndex].width, DGDdata[valueIndex].height,
                                    DGDdata[i].xPos - DGDdata[i].width, DGDdata[i].yPos, DGDdata[i].width, DGDdata[i].height)){
                        values[neighbourDirect].labelValues[lno]--;
                    }
                    break;
                case 2:
                    if (AABBCollision(DGDdata[valueIndex].xLabelTL, DGDdata[valueIndex].yLabelTL, DGDdata[valueIndex].width, DGDdata[valueIndex].height,
                                    DGDdata[i].xPos - DGDdata[i].width, DGDdata[i].yPos + DGDdata[i].height, DGDdata[i].width, DGDdata[i].height)){
                        values[neighbourDirect].labelValues[lno]--;
                    }
                    break;
                case 3:
                    if (AABBCollision(DGDdata[valueIndex].xLabelTL, DGDdata[valueIndex].yLabelTL, DGDdata[valueIndex].width, DGDdata[valueIndex].height,
                                    DGDdata[i].xPos, DGDdata[i].yPos + DGDdata[i].height, DGDdata[i].width, DGDdata[i].height)){
                        values[neighbourDirect].labelValues[lno]--;
                    }
                    break;
                default:
                    break;
                }
            }
            configurationValue += connectedVE.labelValues[currentVE.currentLabel];
        }
    }

    /*Returns the index of the label with the lowest value. If multiple indexes are min the returned index is random*/
    int getSmallestIndex(ValueEntry &vE){
        int index = 0;
        for(int i = 0; i < 4; i++){
            if(vE.labelValues[i] < vE.labelValues[index]){
                index = i;
            }else if(vE.labelValues[i] == vE.labelValues[index]){
                if(rand() > RAND_MAX/2){
                    index = i;
                }
            }
        }
        
        return index;
    }

    /*Sets up all the connections between the neighbouring points. Also initializes the value arrays of all the points depending on their initial random label placement*/
    void setupLabelConnections(std::vector<ValueEntry>& values, std::vector<DGDDataPoint>& DGDdata){
        configurationValue = 0;
        for (int i = 0; i < DGDdata.size(); i++){
            //for all 4 labels check all other possible labels
            for(int lno = 0; lno <4; lno++){
                int Lcount = 0; 
                for (int j = 0; j < DGDdata.size(); j++)
                {
                    if (i != j)
                    {
                        if (DGDdata[j].isLabeled)
                        {
                            switch (lno)
                            {
                            case 0:
                                if (AABBCollision(DGDdata[j].xPos, DGDdata[j].yPos, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos, DGDdata[i].yPos, DGDdata[i].width, DGDdata[i].height) ){
                                    Lcount++;  
                                    values[i].connectedLabels.insert(j);  
                                }else if(AABBCollision(DGDdata[j].xPos - DGDdata[j].width, DGDdata[j].yPos, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos, DGDdata[i].yPos, DGDdata[i].width, DGDdata[i].height) ||
                                                AABBCollision(DGDdata[j].xPos - DGDdata[j].width, DGDdata[j].yPos + DGDdata[j].height, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos, DGDdata[i].yPos, DGDdata[i].width, DGDdata[i].height) ||
                                                AABBCollision(DGDdata[j].xPos, DGDdata[j].yPos + DGDdata[j].height, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos, DGDdata[i].yPos, DGDdata[i].width, DGDdata[i].height))
                                {
                                    values[i].connectedLabels.insert(j);
                                }
                                break;
                            case 1:
                                if (AABBCollision(DGDdata[j].xPos - DGDdata[j].width, DGDdata[j].yPos, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos - DGDdata[i].width, DGDdata[i].yPos, DGDdata[i].width, DGDdata[i].height)){
                                    Lcount++;  
                                    values[i].connectedLabels.insert(j); 
                                }else if (AABBCollision(DGDdata[j].xPos, DGDdata[j].yPos, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos - DGDdata[i].width, DGDdata[i].yPos, DGDdata[i].width, DGDdata[i].height) ||
                                                AABBCollision(DGDdata[j].xPos - DGDdata[j].width, DGDdata[j].yPos + DGDdata[j].height, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos - DGDdata[i].width, DGDdata[i].yPos, DGDdata[i].width, DGDdata[i].height) ||
                                                AABBCollision(DGDdata[j].xPos, DGDdata[j].yPos + DGDdata[j].height, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos - DGDdata[i].width, DGDdata[i].yPos, DGDdata[i].width, DGDdata[i].height))
                                {
                                    values[i].connectedLabels.insert(j);
                                }
                                break;
                            case 2:
                                if(AABBCollision(DGDdata[j].xPos - DGDdata[j].width, DGDdata[j].yPos + DGDdata[j].height, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos - DGDdata[i].width, DGDdata[i].yPos + DGDdata[i].height, DGDdata[i].width, DGDdata[i].height)){
                                    Lcount++;
                                    values[i].connectedLabels.insert(j);
                                }else if (AABBCollision(DGDdata[j].xPos, DGDdata[j].yPos, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos - DGDdata[i].width, DGDdata[i].yPos + DGDdata[i].height, DGDdata[i].width, DGDdata[i].height) ||
                                                AABBCollision(DGDdata[j].xPos - DGDdata[j].width, DGDdata[j].yPos, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos - DGDdata[i].width, DGDdata[i].yPos + DGDdata[i].height, DGDdata[i].width, DGDdata[i].height) ||
                                                AABBCollision(DGDdata[j].xPos, DGDdata[j].yPos + DGDdata[j].height, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos - DGDdata[i].width, DGDdata[i].yPos + DGDdata[i].height, DGDdata[i].width, DGDdata[i].height))
                                {
                                    values[i].connectedLabels.insert(j);
                                }
                                break;
                            case 3:
                                if(AABBCollision(DGDdata[j].xPos, DGDdata[j].yPos + DGDdata[j].height, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos, DGDdata[i].yPos + DGDdata[i].height, DGDdata[i].width, DGDdata[i].height)){
                                    Lcount++;
                                    values[i].connectedLabels.insert(j);
                                }else if (AABBCollision(DGDdata[j].xPos, DGDdata[j].yPos, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos, DGDdata[i].yPos + DGDdata[i].height, DGDdata[i].width, DGDdata[i].height) ||
                                                AABBCollision(DGDdata[j].xPos - DGDdata[j].width, DGDdata[j].yPos, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos, DGDdata[i].yPos + DGDdata[i].height, DGDdata[i].width, DGDdata[i].height) ||
                                                AABBCollision(DGDdata[j].xPos - DGDdata[j].width, DGDdata[j].yPos + DGDdata[j].height, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos, DGDdata[i].yPos + DGDdata[i].height, DGDdata[i].width, DGDdata[i].height))
                                {
                                    values[i].connectedLabels.insert(j);
                                }
                                break;
                            default:
                                break;
                            }
                                                    
                        }
                    }
                }
                values[i].labelValues[lno] = Lcount;
            }
            configurationValue += values[i].labelValues[values[i].currentLabel];
            //std::cout<< "Point " << i <<", with label "<< values[i].currentLabel <<" has values: L0 = " << values[i].labelValues[0]<<", L1 = "<<values[i].labelValues[1]<<", L2 = " <<values[i].labelValues[2]<<", L3 = "<<values[i].labelValues[3]<<std::endl;
        }
    }

    /*Updates the label values of all the neighbours of the point at valueIndex after its label was moved.*/
    void updateLocalLabels(std::vector<ValueEntry>& values, std::vector<DGDDataPoint>& DGDdata, int valueIndex, int oldXTL, int oldYTL){
        //currentVE is the just updated point with its label
        ValueEntry currentVE = values[valueIndex];
        //For all neighboring labels, update their values (using their connected labels)
        for (const auto& neighbourDirect: currentVE.connectedLabels) {
            ValueEntry connectedVE = values[neighbourDirect];
            configurationValue -= connectedVE.labelValues[currentVE.currentLabel];
            int i = neighbourDirect;
            for(int lno = 0; lno <4; lno++){
                switch (lno)
                {
                    //check if there was a collision with the old label (remove 1 value) or a collision with the new label (add 1 value) for all four labels of the neighbour
                case 0:
                    if (AABBCollision(oldXTL, oldYTL, DGDdata[valueIndex].width, DGDdata[valueIndex].height,
                                        DGDdata[i].xPos, DGDdata[i].yPos, DGDdata[i].width, DGDdata[i].height))
                        {
                            if(values[neighbourDirect].labelValues[lno] > 0){
                                values[neighbourDirect].labelValues[lno]--;
                            }
                        }
                    if (AABBCollision(DGDdata[valueIndex].xLabelTL, DGDdata[valueIndex].yLabelTL, DGDdata[valueIndex].width, DGDdata[valueIndex].height,
                                    DGDdata[i].xPos, DGDdata[i].yPos, DGDdata[i].width, DGDdata[i].height))
                    {
                        values[neighbourDirect].labelValues[lno]++;
                    }
                   
                    break;
                case 1:
                    if (AABBCollision(oldXTL, oldYTL, DGDdata[valueIndex].width, DGDdata[valueIndex].height,
                                    DGDdata[i].xPos - DGDdata[i].width, DGDdata[i].yPos, DGDdata[i].width, DGDdata[i].height))
                    {
                        if(values[neighbourDirect].labelValues[lno] > 0){
                            values[neighbourDirect].labelValues[lno]--;
                        }
                    }
                    if (AABBCollision(DGDdata[valueIndex].xLabelTL, DGDdata[valueIndex].yLabelTL, DGDdata[valueIndex].width, DGDdata[valueIndex].height,
                                    DGDdata[i].xPos - DGDdata[i].width, DGDdata[i].yPos, DGDdata[i].width, DGDdata[i].height))
                    {
                        values[neighbourDirect].labelValues[lno]++;
                    }
                    break;
                case 2:
                    if (AABBCollision(oldXTL, oldYTL, DGDdata[valueIndex].width, DGDdata[valueIndex].height,
                                    DGDdata[i].xPos - DGDdata[i].width, DGDdata[i].yPos + DGDdata[i].height, DGDdata[i].width, DGDdata[i].height))
                    {
                        if(values[neighbourDirect].labelValues[lno] > 0){
                            values[neighbourDirect].labelValues[lno]--;
                        }
                    }
                    if (AABBCollision(DGDdata[valueIndex].xLabelTL, DGDdata[valueIndex].yLabelTL, DGDdata[valueIndex].width, DGDdata[valueIndex].height,
                                    DGDdata[i].xPos - DGDdata[i].width, DGDdata[i].yPos + DGDdata[i].height, DGDdata[i].width, DGDdata[i].height))
                    {
                        values[neighbourDirect].labelValues[lno]++;
                    }
                    break;
                case 3:
                    if (AABBCollision(oldXTL, oldYTL, DGDdata[valueIndex].width, DGDdata[valueIndex].height,
                                    DGDdata[i].xPos, DGDdata[i].yPos + DGDdata[i].height, DGDdata[i].width, DGDdata[i].height))
                    {
                        if(values[neighbourDirect].labelValues[lno] > 0){
                            values[neighbourDirect].labelValues[lno]--;
                        }
                    }
                    if (AABBCollision(DGDdata[valueIndex].xLabelTL, DGDdata[valueIndex].yLabelTL, DGDdata[valueIndex].width, DGDdata[valueIndex].height,
                                    DGDdata[i].xPos, DGDdata[i].yPos + DGDdata[i].height, DGDdata[i].width, DGDdata[i].height))
                    {
                        values[neighbourDirect].labelValues[lno]++;
                    }
                    break;
                default:
                    break;
                }
            }
            configurationValue += connectedVE.labelValues[currentVE.currentLabel];
        }
    }

    /*void calculateLabelValues(std::vector<ValueEntry>& values, std::vector<DataPoint>& DGDdata){
        configurationValue = 0;
        for (int i = 0; i < DGDdata.size(); i++){
                //calculate value of all 4 positions
                int Lcount = 0;

            for(int lno = 0; lno <4; lno++){
                Lcount = 0; 
                for (int j = 0; j < DGDdata.size(); j++)
                {
                    if (i != j)
                    {
                        if (DGDdata[j].isLabeled)
                        {
                            switch (lno)
                            {
                            case 0:
                                if (AABBCollision(DGDdata[j].xLabelTL, DGDdata[j].yLabelTL, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos, DGDdata[i].yPos, DGDdata[i].width, DGDdata[i].height))
                                {
                                    Lcount++;
                                }
                                break;
                            case 1:
                                if (AABBCollision(DGDdata[j].xLabelTL, DGDdata[j].yLabelTL, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos - DGDdata[i].width, DGDdata[i].yPos, DGDdata[i].width, DGDdata[i].height))
                                {
                                    Lcount++;
                                }
                                break;
                            case 2:
                                if (AABBCollision(DGDdata[j].xLabelTL, DGDdata[j].yLabelTL, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos - DGDdata[i].width, DGDdata[i].yPos + DGDdata[i].height, DGDdata[i].width, DGDdata[i].height))
                                {
                                    Lcount++;
                                }
                                break;
                            case 3:
                                if (AABBCollision(DGDdata[j].xLabelTL, DGDdata[j].yLabelTL, DGDdata[j].width, DGDdata[j].height,
                                                DGDdata[i].xPos, DGDdata[i].yPos + DGDdata[i].height, DGDdata[i].width, DGDdata[i].height))
                                {
                                    Lcount++;
                                }
                                break;
                            default:
                                break;
                            }
                                                    
                        }
                    }
                }
                values[i].labelValues[lno] = Lcount;
            }
           // std::cout<< "Point " << i <<" has values: L0 = " << values[i].labelValues[0]<<", L1 = "<<values[i].labelValues[1]<<", L2 = " <<values[i].labelValues[2]<<", L3 = "<<values[i].labelValues[3]<<std::endl;
            configurationValue += values[i].labelValues[values[i].currentLabel];
        }

    }*/
    inline bool AABBCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2){
        return x1 < (x2 + w2) && (x1 + w1) > x2 && (y1 - h1) < y2 && ((y1 - h1) + h1) > (y2 -h2);
    }
};