#include <iostream>
#include <vector>
#include <random>
#include <limits>
#include <set>
#include "file.h"
#include "record.h"
#include "buffer.h"
#include "test.h"

using namespace std;

struct ProgramInfo{
    string filename;
    bool printing;
    bool testing;
    int b;
    ProgramInfo(string f, bool p, int blockingFactor, bool test){
        filename = f;
        printing = p;
        b = blockingFactor;
        testing = test;
    }

};

template <typename T>
void copyFile(Buffer<T> &buffer1, Buffer<T> &buffer2){
    while(auto record = buffer2.next()){
        buffer1.write(*record);
    }
    buffer1.flush();
}

pair<int, int> getFib(int n){
    pair<int,int> fib = {1, 0};

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
pair<int ,int> divide(vector<File<T>> &tapes, vector<Buffer<T>> &buffers, ProgramInfo &settings){
    int runs = countRuns(buffers[0]);
    //cout<<runs<<"\n";
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

    if(settings.printing){
        cout<<"INITIAL DISTRIBUTION:\n\n";
        printFiles(buffers);
        cout<<"\n";
    }

    //cout<<runs<<" "<<fib.first<<" "<<fib.second<<"\n";

    return fib;
}

template <typename T>
int merge(vector<File<T>> &tapes, vector<Buffer<T>> &buffers, pair<int, int> fib, ProgramInfo &settings){
    int C = 0;
    int A = 1;
    int B = 2;

    int phaseCounter = 0;

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
        phaseCounter++;
        if(settings.printing){
            cout<<"PHASE "<<phaseCounter<<":\n\n";
            printFiles(buffers);
            cout<<"\n";
        }
    }

    if(B != 0){
        tapes[B].remove();
    }
    if(C != 0){
        tapes[C].remove();
    }

    buffers[A].reset();
    if(A != 0){
        tapes[0].clear();
        buffers[0].reset();
        copyFile(buffers[0], buffers[A]);
        tapes[A].remove();
        buffers[0].reset();
    }

    return phaseCounter;

}

template <typename T>
void printFiles(vector<Buffer<T>> &buffers){
    for(auto &buffer : buffers){
        buffer.print();
        cout<<"\n";
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

string pickOption(string prompt, vector<string> options){
    string answer;
    string op = "(" + options[0];
    for(int i=1;i<options.size();i++){
        op +=  "/" + options[i];
    }
    op += "): ";

    while(true){
        cout<<prompt<<" "<<op;
        cin>>answer;
        if(find(options.begin(), options.end(), answer) != options.end()){
            return answer;
        }
    }
}

int getIntInput(string prompt){
    int value;
    while(true){
        cout<<prompt<<": ";
        if(cin >> value && value > 0){
            return value;
        }
        cout<<"Invalid input\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

ProgramInfo menu(){
    string answer = "", filename = "";

    int b = getIntInput("Set your blocking factor (b)");

    bool existingFile = pickOption("Do you want to read data from an existing file?", {"Y", "N"}) == "Y";   
    if(existingFile){
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
        bool generate = pickOption("Do you want to generate random data or type the records yourself?", {"1", "2"}) == "1";

        if(generate){
            int N = getIntInput("Number of records to generate");
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
    bool angle = pickOption("Do you want to sort records by angle or by radius?", {"1", "2"}) == "1";
    Record::sortField = angle ? ANGLE : RADIUS;
    bool printing = pickOption("Do you want to print your files after every phase?", {"Y", "N"}) == "Y";
    bool testing = pickOption("Do you want to check if your file was sorted correctly?", {"Y", "N"}) == "Y";
    if(printing){
        cout<<"Here is your file:\n";
        fstream f(filename);
        double a, r;
        while(f >> a >> r){
            cout<<a<<" "<<r<<"\n";
        }
        f.close();
    }
    cout<<"\n";
    return ProgramInfo(filename, printing, b, testing);
}

int main(){

    ProgramInfo settings = menu();

    int b = settings.b;
    string filename1 = settings.filename;

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

    Test<Record> test;
    if(settings.testing){
        test = Test<Record>(filename1);
    }

    cout<<"\n--- SORTING BEGIN ---\n";
    pair<int, int> fibDivision = divide(tapes, buffers, settings);
    int phases = merge(tapes, buffers, fibDivision, settings);

    cout<<"\n--- SORTING COMPLETE ---\n";
    if (settings.testing) {
        bool ok = test.check(filename1);
        cout << (ok ? "TEST PASSED\n" : "TEST FAILED\n");
    }

    cout<<"READS: "<<Buffer<Record>::reads<<"\n";
    cout<<"WRITES: "<<Buffer<Record>::writes<<"\n";
    cout<<"PHASES: "<<phases;

    return 0;
}