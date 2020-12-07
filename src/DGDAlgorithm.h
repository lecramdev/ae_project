#pragma once

#include "Algorithm.h"
#include "util.h"
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
    void run(std::vector<DataPoint>& data) override
    {
        //Step 1: All Labels are assigned a random position
        //|--------|--------|
        //|    2   |    3   |
        //|--------X--------|
        //|    1   |    0   |
        //|--------|--------|
        srand (time(NULL));
        std::vector<ValueEntry> values;
        values.reserve(data.size());
        for (int i = 0; i < data.size(); i++)
        {
            values.emplace_back(0,0,0,0,-1);
            int randpos = rand() % 4;
            data[i].isLabeled = true;
            applyLabelToPos(values, data, i, randpos);
            
        }
        //Step 2 set up all label position values:
        setupLabelConnections(values, data);
        
        //step 3: until no further improvement is possible:
        int countdown = 3;
        int lastValue = 1;
        while(countdown >= 0){
            
            //a) for each label consider the best position and move it there
            for (int i = 0; i < values.size(); i++)
            {
                int oldPos = values[i].currentLabel;
                int oldxTL = data[i].xLabelTL;
                int oldyTL = data[i].yLabelTL;
                int newPos = getSmallestIndex(values[i]);
                applyLabelToPos(values, data, i, newPos);
                                
                //update total configurationValue
                configurationValue -= values[i].labelValues[oldPos];
                configurationValue += values[i].labelValues[newPos];
                //update the values of the surrounding labels
                updateLocalLabels(values, data, i, oldxTL, oldyTL);
                
                
            }
            if(lastValue == configurationValue){
                countdown--;
            }else if(lastValue < configurationValue + 2 && lastValue > configurationValue - 2){
                countdown --;
            }else{
                countdown == 3;
            }
            std::cout<<configurationValue<<std::endl;
            lastValue = configurationValue;
        }

        //delete all overlapping labels
        for (int i = 0; i < values.size(); i++)
        {
            if(values[i].labelValues[values[i].currentLabel] > 0){
                configurationValue -= values[i].labelValues[values[i].currentLabel];
                deleteLabel(values, data, i);
                data[i].isLabeled = false;
                data[i].xLabelTL = 0;
                data[i].yLabelTL = 0;
            }
            //std::cout<< "Point " << i <<", with label "<< values[i].currentLabel <<" has values: L0 = " << values[i].labelValues[0]<<", L1 = "<<values[i].labelValues[1]<<", L2 = " <<values[i].labelValues[2]<<", L3 = "<<values[i].labelValues[3]<<std::endl;
        
        }
        
        
        
    }


    /*Updates the values in the data vector depending on the given position */
    void applyLabelToPos(std::vector<ValueEntry>& values, std::vector<DataPoint>& data, int i, int pos){
        switch(pos) {
            case 0:
                data[i].xLabelTL = data[i].xPos;
                data[i].yLabelTL = data[i].yPos;
                values[i].currentLabel = 0; 
                break;
            case 1:
                data[i].xLabelTL = data[i].xPos - data[i].width;
                data[i].yLabelTL = data[i].yPos;
                values[i].currentLabel = 1; 
                break;
            case 2:
                data[i].xLabelTL = data[i].xPos - data[i].width;
                data[i].yLabelTL = data[i].yPos + data[i].height;
                values[i].currentLabel = 2; 
                break;
            case 3:
                data[i].xLabelTL = data[i].xPos;
                data[i].yLabelTL = data[i].yPos + data[i].height;
                values[i].currentLabel = 3; 
                break;
            default:
                break;
        }
    }

    /*Updates all Label values in the Value array after the Label in value index was deleted*/
    void deleteLabel(std::vector<ValueEntry>& values, std::vector<DataPoint>& data, int valueIndex){
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
                    if (AABBCollision(data[valueIndex].xLabelTL, data[valueIndex].yLabelTL, data[valueIndex].width, data[valueIndex].height,
                                    data[i].xPos, data[i].yPos, data[i].width, data[i].height)){
                        values[neighbourDirect].labelValues[lno]--;
                    }
                    break;
                case 1:
                    if (AABBCollision(data[valueIndex].xLabelTL, data[valueIndex].yLabelTL, data[valueIndex].width, data[valueIndex].height,
                                    data[i].xPos - data[i].width, data[i].yPos, data[i].width, data[i].height)){
                        values[neighbourDirect].labelValues[lno]--;
                    }
                    break;
                case 2:
                    if (AABBCollision(data[valueIndex].xLabelTL, data[valueIndex].yLabelTL, data[valueIndex].width, data[valueIndex].height,
                                    data[i].xPos - data[i].width, data[i].yPos + data[i].height, data[i].width, data[i].height)){
                        values[neighbourDirect].labelValues[lno]--;
                    }
                    break;
                case 3:
                    if (AABBCollision(data[valueIndex].xLabelTL, data[valueIndex].yLabelTL, data[valueIndex].width, data[valueIndex].height,
                                    data[i].xPos, data[i].yPos + data[i].height, data[i].width, data[i].height)){
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
    void setupLabelConnections(std::vector<ValueEntry>& values, std::vector<DataPoint>& data){
        configurationValue = 0;
        for (int i = 0; i < data.size(); i++){
            //for all 4 labels check all other possible labels
            for(int lno = 0; lno <4; lno++){
                int Lcount = 0; 
                for (int j = 0; j < data.size(); j++)
                {
                    if (i != j)
                    {
                        if (data[j].isLabeled)
                        {
                            switch (lno)
                            {
                            case 0:
                                if (AABBCollision(data[j].xPos, data[j].yPos, data[j].width, data[j].height,
                                                data[i].xPos, data[i].yPos, data[i].width, data[i].height) ){
                                    Lcount++;  
                                    values[i].connectedLabels.insert(j);  
                                }else if(AABBCollision(data[j].xPos - data[j].width, data[j].yPos, data[j].width, data[j].height,
                                                data[i].xPos, data[i].yPos, data[i].width, data[i].height) ||
                                                AABBCollision(data[j].xPos - data[j].width, data[j].yPos + data[j].height, data[j].width, data[j].height,
                                                data[i].xPos, data[i].yPos, data[i].width, data[i].height) ||
                                                AABBCollision(data[j].xPos, data[j].yPos + data[j].height, data[j].width, data[j].height,
                                                data[i].xPos, data[i].yPos, data[i].width, data[i].height))
                                {
                                    values[i].connectedLabels.insert(j);
                                }
                                break;
                            case 1:
                                if (AABBCollision(data[j].xPos - data[j].width, data[j].yPos, data[j].width, data[j].height,
                                                data[i].xPos - data[i].width, data[i].yPos, data[i].width, data[i].height)){
                                    Lcount++;  
                                    values[i].connectedLabels.insert(j); 
                                }else if (AABBCollision(data[j].xPos, data[j].yPos, data[j].width, data[j].height,
                                                data[i].xPos - data[i].width, data[i].yPos, data[i].width, data[i].height) ||
                                                AABBCollision(data[j].xPos - data[j].width, data[j].yPos + data[j].height, data[j].width, data[j].height,
                                                data[i].xPos - data[i].width, data[i].yPos, data[i].width, data[i].height) ||
                                                AABBCollision(data[j].xPos, data[j].yPos + data[j].height, data[j].width, data[j].height,
                                                data[i].xPos - data[i].width, data[i].yPos, data[i].width, data[i].height))
                                {
                                    values[i].connectedLabels.insert(j);
                                }
                                break;
                            case 2:
                                if(AABBCollision(data[j].xPos - data[j].width, data[j].yPos + data[j].height, data[j].width, data[j].height,
                                                data[i].xPos - data[i].width, data[i].yPos + data[i].height, data[i].width, data[i].height)){
                                    Lcount++;
                                    values[i].connectedLabels.insert(j);
                                }else if (AABBCollision(data[j].xPos, data[j].yPos, data[j].width, data[j].height,
                                                data[i].xPos - data[i].width, data[i].yPos + data[i].height, data[i].width, data[i].height) ||
                                                AABBCollision(data[j].xPos - data[j].width, data[j].yPos, data[j].width, data[j].height,
                                                data[i].xPos - data[i].width, data[i].yPos + data[i].height, data[i].width, data[i].height) ||
                                                AABBCollision(data[j].xPos, data[j].yPos + data[j].height, data[j].width, data[j].height,
                                                data[i].xPos - data[i].width, data[i].yPos + data[i].height, data[i].width, data[i].height))
                                {
                                    values[i].connectedLabels.insert(j);
                                }
                                break;
                            case 3:
                                if(AABBCollision(data[j].xPos, data[j].yPos + data[j].height, data[j].width, data[j].height,
                                                data[i].xPos, data[i].yPos + data[i].height, data[i].width, data[i].height)){
                                    Lcount++;
                                    values[i].connectedLabels.insert(j);
                                }else if (AABBCollision(data[j].xPos, data[j].yPos, data[j].width, data[j].height,
                                                data[i].xPos, data[i].yPos + data[i].height, data[i].width, data[i].height) ||
                                                AABBCollision(data[j].xPos - data[j].width, data[j].yPos, data[j].width, data[j].height,
                                                data[i].xPos, data[i].yPos + data[i].height, data[i].width, data[i].height) ||
                                                AABBCollision(data[j].xPos - data[j].width, data[j].yPos + data[j].height, data[j].width, data[j].height,
                                                data[i].xPos, data[i].yPos + data[i].height, data[i].width, data[i].height))
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
    void updateLocalLabels(std::vector<ValueEntry>& values, std::vector<DataPoint>& data, int valueIndex, int oldXTL, int oldYTL){
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
                    if (AABBCollision(oldXTL, oldYTL, data[valueIndex].width, data[valueIndex].height,
                                        data[i].xPos, data[i].yPos, data[i].width, data[i].height))
                        {
                            if(values[neighbourDirect].labelValues[lno] > 0){
                                values[neighbourDirect].labelValues[lno]--;
                            }
                        }
                    if (AABBCollision(data[valueIndex].xLabelTL, data[valueIndex].yLabelTL, data[valueIndex].width, data[valueIndex].height,
                                    data[i].xPos, data[i].yPos, data[i].width, data[i].height))
                    {
                        values[neighbourDirect].labelValues[lno]++;
                    }
                   
                    break;
                case 1:
                    if (AABBCollision(oldXTL, oldYTL, data[valueIndex].width, data[valueIndex].height,
                                    data[i].xPos - data[i].width, data[i].yPos, data[i].width, data[i].height))
                    {
                        if(values[neighbourDirect].labelValues[lno] > 0){
                            values[neighbourDirect].labelValues[lno]--;
                        }
                    }
                    if (AABBCollision(data[valueIndex].xLabelTL, data[valueIndex].yLabelTL, data[valueIndex].width, data[valueIndex].height,
                                    data[i].xPos - data[i].width, data[i].yPos, data[i].width, data[i].height))
                    {
                        values[neighbourDirect].labelValues[lno]++;
                    }
                    break;
                case 2:
                    if (AABBCollision(oldXTL, oldYTL, data[valueIndex].width, data[valueIndex].height,
                                    data[i].xPos - data[i].width, data[i].yPos + data[i].height, data[i].width, data[i].height))
                    {
                        if(values[neighbourDirect].labelValues[lno] > 0){
                            values[neighbourDirect].labelValues[lno]--;
                        }
                    }
                    if (AABBCollision(data[valueIndex].xLabelTL, data[valueIndex].yLabelTL, data[valueIndex].width, data[valueIndex].height,
                                    data[i].xPos - data[i].width, data[i].yPos + data[i].height, data[i].width, data[i].height))
                    {
                        values[neighbourDirect].labelValues[lno]++;
                    }
                    break;
                case 3:
                    if (AABBCollision(oldXTL, oldYTL, data[valueIndex].width, data[valueIndex].height,
                                    data[i].xPos, data[i].yPos + data[i].height, data[i].width, data[i].height))
                    {
                        if(values[neighbourDirect].labelValues[lno] > 0){
                            values[neighbourDirect].labelValues[lno]--;
                        }
                    }
                    if (AABBCollision(data[valueIndex].xLabelTL, data[valueIndex].yLabelTL, data[valueIndex].width, data[valueIndex].height,
                                    data[i].xPos, data[i].yPos + data[i].height, data[i].width, data[i].height))
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

    /*void calculateLabelValues(std::vector<ValueEntry>& values, std::vector<DataPoint>& data){
        configurationValue = 0;
        for (int i = 0; i < data.size(); i++){
                //calculate value of all 4 positions
                int Lcount = 0;

            for(int lno = 0; lno <4; lno++){
                Lcount = 0; 
                for (int j = 0; j < data.size(); j++)
                {
                    if (i != j)
                    {
                        if (data[j].isLabeled)
                        {
                            switch (lno)
                            {
                            case 0:
                                if (AABBCollision(data[j].xLabelTL, data[j].yLabelTL, data[j].width, data[j].height,
                                                data[i].xPos, data[i].yPos, data[i].width, data[i].height))
                                {
                                    Lcount++;
                                }
                                break;
                            case 1:
                                if (AABBCollision(data[j].xLabelTL, data[j].yLabelTL, data[j].width, data[j].height,
                                                data[i].xPos - data[i].width, data[i].yPos, data[i].width, data[i].height))
                                {
                                    Lcount++;
                                }
                                break;
                            case 2:
                                if (AABBCollision(data[j].xLabelTL, data[j].yLabelTL, data[j].width, data[j].height,
                                                data[i].xPos - data[i].width, data[i].yPos + data[i].height, data[i].width, data[i].height))
                                {
                                    Lcount++;
                                }
                                break;
                            case 3:
                                if (AABBCollision(data[j].xLabelTL, data[j].yLabelTL, data[j].width, data[j].height,
                                                data[i].xPos, data[i].yPos + data[i].height, data[i].width, data[i].height))
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
};
