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
    ifstream reader;
    ofstream writer;
    int mode;
    int linesRead = 0;

public:
    File(string filename){
        this->filename = filename;
        mode = IDLE;
        createFile();
    }

    void createFile(){
        fstream file(filename);
        if(!file.is_open()){
            ofstream create(filename);
            create.close();
        }
        else{
            file.close();
        }
    }

    void remove(){
        if(mode != IDLE){
            changeMode(IDLE);
        }
        filesystem::remove(filename);
    }
    
    void clear(){
        if(mode != IDLE){
            changeMode(IDLE);
        }
        ofstream clearing(filename);
        clearing.close();
    }

    vector<T> getRecords(int n){
        if(mode != READ){
            changeMode(READ);
        }

        vector<T> records;
        T record;
        while(records.size() < n && reader >> record){
            records.push_back(record);
        }

        return records;
    }

    void write(vector<T> &records){
        if(mode != WRITE){
            changeMode(WRITE);
        }
        for(T record : records){
            writer << record << "\n";
        }
    }

    void changeMode(int m){
        if(mode == READ){
            reader.close();
        }
        else if(mode == WRITE){
            writer.close();
        }
        mode = m;
        if(mode == READ){
            reader.open(filename, ios::binary);
        }
        else if(mode == WRITE){
            writer.open(filename, ios::app);
        }
    }
    
    void close(){
        if(mode != IDLE){
            changeMode(IDLE);
        }
    }

    string getName(){
        return filename;
    }

    void print(){
        if(mode == READ){
            streampos position = reader.tellg();
            T record;  
            while(reader >> record){
                cout<<record<<"\n";
            }
            reader.close();
            reader.open(filename, ios::binary);
            reader.seekg(position);
        }
        else{
            if(mode != IDLE){
                changeMode(IDLE);
            }
            fstream temp(filename);
            T record;
            while(temp >> record){
                cout<<record<<"\n";
            }
            temp.close();
        }
    }
};
