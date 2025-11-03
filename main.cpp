#include <iostream>
#include <vector>
#include <random>
#include "file.h"
#include "record.h"
#include "buffer.h"

using namespace std;

pair<int, int> getFib(int n){
    pair<int,int> fib = {1, 1};

    while(fib.first + fib.second < n){
        fib = {fib.first + fib.second, fib.first};
    }

    return fib;
}

template <typename T>
int countRuns(Buffer<T> &buff){
    auto temp = buff.next();
    if(!temp){
        return 0;
    }

    T previous = *temp;
    int counter = 1;

    while(auto temp = buff.next()){
        T record = *temp;
        if(record < previous){
            counter++;  
        }
        previous = record;
    }
    return counter;
}

template <typename T>
pair<int ,int> divide(vector<File<T>> &tapes, vector<Buffer<T>> &buffers){
    int runs = countRuns(buffers[0]);
    cout<<runs<<"\n";
    buffers[0].reset();

    pair<int, int> fib = getFib(runs);
    
    int runCounter = 0;

    bool first = true;
    int currTape = 1;
    T previous;
    while(auto temp = buffers[0].next()){
        T record = *temp;
        if(first){
            previous = record;
            first = false;
        }
        if(record < previous){
            runCounter++;
            if(runCounter == fib.first){
                currTape++;
            }
        }
        buffers[currTape].write(record);
        previous = record;
    }
    tapes[0].clear();
    buffers[1].flush();
    buffers[2].flush();
    for(int i=0;i<3;i++){
        buffers[i].reset();
    }
    return fib;
}

template <typename T>
void merge(vector<File<T>> &tapes, vector<Buffer<T>> &buffers, pair<int, int> fib){
    int C = 0;
    int A = 1;
    int B = 2;

    int phaseCounter = 1;

    while(fib.second > 0){
        bool endOfFileB = false;
        for(int i=0;i<fib.second;i++){
            //cout<<i+1<<".\n";
            bool endA = false, endB = endOfFileB;
            optional<T> recA = buffers[A].next();
            T prevA = *recA, prevB;
            optional<T> recB;
            if(!endOfFileB){
                recB = buffers[B].next();
            }
            if(recB){
                T prevB = *recB;
            }
            else{
                endB = true;
            }

            while(!endA || !endB){
                if(!endA && (endB || *recA < *recB)){
                    buffers[C].write(*recA);
                    prevA = *recA;
                    recA = buffers[A].next();
                    if(!recA || *recA < prevA){
                        if(recA){
                            buffers[A].reRead();
                        }
                        endA = true;
                    }
                }
                else{
                    buffers[C].write(*recB);
                    prevB = *recB;
                    recB = buffers[B].next();
                    if(!recB || *recB < prevB){
                        if(recB){
                            buffers[B].reRead();
                        }
                        else{
                            endOfFileB = true;
                        }
                        endB = true;
                    }
                }
            }
        }

        buffers[C].flush();
        buffers[C].reset();
        buffers[B].reset();
        tapes[B].clear();

        A = (A + 2) % 3;
        B = (B + 2) % 3;
        C = (C + 2) % 3;

        fib = {fib.second, fib.first - fib.second};
        cout<<"\nPHASE "<<phaseCounter++<<":\n\n";
        printFiles(buffers);
        cout<<"\n";
    }

    tapes[B].remove();
    tapes[C].remove();
    buffers[A].reset();

}

template <typename T>
void printFiles(vector<Buffer<T>> &buffers){
    for(auto &buffer : buffers){
        buffer.print();
    }
}

void generateData(int N, string filename){
    random_device rd;
    mt19937_64 gen(rd());
    uniform_real_distribution<double> angle(0, 360);
    uniform_real_distribution<double> radius(1, 100);

    ofstream file;
    file.open(filename);

    for(int i = 0; i < N; i++){
        file << angle(gen) << " " << radius(gen) << "\n";
    }

    file.close();
}

pair<int, string> menu(){
    string answer = "", filename = "";
    int b;

    cout<<"What is your desired value for the blocking factor (b)?\n";
    cin>>b;

    while(answer != "Y" && answer != "N"){
        cout<<"Do you want to read data from an existing file? (Y/N)\n";
        cin>>answer;
    }
    if(answer == "Y"){
        while(true){
            cout<<"Input the name of the file: ";
            cin>>filename;
            fstream test(filename);
            if(test.is_open()){
                test.close();
                break;
            }
            cout<<"Wasn't able to open file "<<filename<<"\n";
        }
    }
    else{
        filename = "input.txt";
        answer = "";
        while(answer != "1" && answer != "2"){
            cout<<"Do you want to generate random data (1) or type the records yourself (2)?\n";
            cin>>answer;
        }

        if(answer == "1"){
            int N;
            cout<<"How many records do you want to generate?\n";
            cin>>N;
            generateData(N, filename);
        }
        else{
            cout<<"Input your records (one record is two doubles seperated by a space), to stop writing, enter -1\n\n";
            ofstream file(filename);
            
            double a, r;

            while(true){
                cin>>a;
                if(a == -1){
                    break;
                }
                cin>>r;
                file<<a<<" "<<r<<"\n";
            }

            file.close();
        }
    }


    cout<<"Here is your file:\n";
    fstream f(filename);
    double a, r;
    while(f >> a >> r){
        cout<<a<<" "<<r<<"\n";
    }
    f.close();

    return {b, filename};
}

int main(){

    pair<int, string> settings = menu();

    int b = settings.first;
    string filename1 = settings.second;

    string filename2 = "tape2.txt";
    string filename3 = "tape3.txt";

    vector<File<Record>> tapes;
    tapes.emplace_back(filename1);
    tapes.emplace_back(filename2);
    tapes.emplace_back(filename3);
    
    vector<Buffer<Record>> buffers;
    for(int i=0;i<tapes.size();i++){
        buffers.push_back(Buffer<Record>(tapes[i], b));
    }

    pair<int, int> fibDivision = divide(tapes, buffers);
    printFiles(buffers);
    merge(tapes, buffers, fibDivision);

    cout<<"READS: "<<Buffer<Record>::reads<<"\n";
    cout<<"WRITES: "<<Buffer<Record>::writes;


    return 0;
}