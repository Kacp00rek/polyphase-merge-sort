#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

template <typename T>
class Test{
    vector<T> data;

public:
    Test(){

    }
    Test(string input){
        fstream file(input);
        if(!file.is_open()){
            cout<<"Couldn't open file "<<input<<" for testing\n";
            return;
        }
        T record;
        while(file >> record){
            data.push_back(record);
        }
        file.close();
        sort(data.begin(), data.end());
    }
    bool check(string output){
        int i = 0;
        bool answer = true;
        fstream file(output);
        if(!file.is_open()){
            cout<<"Couldn't open file "<<output<<" for testing\n";
            return false;
        }
        T record;
        while(file >> record){
            if(i >= data.size()){
                cout<<"Too much data in the output file\n";
                answer = false;
                break;
            }
            if(record.getField() != data[i].getField()){
                cout<<"Incorrect record in line "<<i+1<<"\n";
                cout<<record<<" vs "<<data[i]<<"\n";
                answer = false;
                break;
            }
            i++;
        }
        if(answer && i != data.size()){
            cout<<"Too little data in the output file\n";
            answer = false;
        }
        return answer;
    }
};
