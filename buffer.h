#pragma once
#include <vector>
#include <optional>
#include "file.h"

using namespace std;

template <typename T>
class Buffer {
    File<T> &file;
    int capacity; 
    int currRecord;
    bool end;
    vector<T> records;

public:
    static int reads;
    static int writes;

    Buffer(File<T> &f, int cap) : file(f){
        capacity = cap;
        currRecord = 0;
        end = false;
    }

    optional<T> next(){
        int previous_size = records.size();
        if(currRecord >= records.size()){
            if(end){
                return nullopt;
            }
            records = file.getRecords(capacity);
            currRecord = 0;
            if(records.size() < capacity){
                end = true;
            }
            reads++;
        }
        if(records.empty()){
            return nullopt;
        }

        return records[currRecord++];
    }

    void write(T record){
        records.push_back(record);
        if(records.size() == capacity){
            flush();
        }
    }

    void reset(){
        file.close();
        end = false;
        records.clear();
        currRecord = 0;
    }

    void print(){
        cout<<file.getName()<<":\n";
        for(int i = currRecord; i<records.size(); i++){
            cout<<records[i]<<"\n";
        }
        file.print();
    }

    void flush(){
        if(records.size() > 0){
            writes++;
            file.write(records);
            records.clear();
        }
    }
    
    void reRead(){
        currRecord--;
    }
};
template<typename T>
int Buffer<T>::reads = 0;
template<typename T>
int Buffer<T>::writes = 0;