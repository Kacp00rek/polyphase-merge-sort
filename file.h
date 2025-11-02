#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>

#define IDLE    -1
#define READ    0
#define WRITE   1

using namespace std;

template <typename T>
class File {
    string filename;
    fstream file;
    int mode;

public:
    File(string filename){
        this->filename = filename;
        mode = IDLE;
        file.open(filename);
        if(!file.is_open()){
            ofstream create(filename);
            create.close();
        }
        else{
            file.close();
        }
    }

    void remove(){
        if(file.is_open()){
            file.close();
        }
        filesystem::remove(filename);
    }
    
    void clear(){
        ofstream clearing(filename);
        clearing.close();
    }

    vector<T> getRecords(int n){
        if(mode != READ){
            changeMode(READ);
        }

        vector<T> records;
        T record;
        while(records.size() < n && file >> record){
            records.push_back(record);
        }

        return records;
    }

    void write(vector<T> &records){
        if(mode != WRITE){
            changeMode(WRITE);
        }
        for(T record : records){
            file << record << "\n";
        }
    }

    void changeMode(int m){
        mode = m;
        if(file.is_open()){
            file.close();
        }
        file.open(filename);
    }
    
    void close(){
        if(file.is_open()){
            file.close();
        }
        mode = IDLE;
    }

    string getName(){
        return filename;
    }

    void print(){
        cout<<filename<<":\n";
        if(mode == READ){
            streampos position = file.tellg();
            T record;
            
            while(file >> record){
                cout<<record<<"\n";
            }

            file.close();
            file.open(filename);
            file.seekg(position);
        }
        else{
            if(file.is_open()){
                file.close();
            }
            fstream temp(filename);
            T record;
            while(temp >> record){
                cout<<record<<"\n";
            }
            temp.close();
        }
        cout<<"\n";
    }

};
