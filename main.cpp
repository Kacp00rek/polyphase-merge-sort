#include <iostream>
#include <vector>
#include <random>
#include <limits>
#include <set>
#include "file.h"
#include "record.h"
#include "buffer.h"
#include "test.h"

#define SORTBEGIN   cout<<"\n--- SORTING BEGIN ---\n"
#define SORTEND     cout<<"\n--- SORTING COMPLETE ---\n"

using namespace std;
using T = Record;

struct ProgramInfo{
    string filename;
    bool printing;
    optional<Test<T>> test;
    int b;
    ProgramInfo(string f, bool p, int blockingFactor, optional<Test<T>> t){
        filename = f;
        printing = p;
        b = blockingFactor;
        test = t;
    }
};

struct SetupResult{
    ProgramInfo settings;
    vector<File<T>> tapes;
    vector<Buffer<T>> buffers;
};

pair<int, int> getFib(int n){
    pair<int,int> fib = {1, 0};

    while(fib.first + fib.second < n){
        fib = {fib.first + fib.second, fib.first};
    }

    return fib;
}

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

void printFiles(vector<Buffer<T>> &buffers){
    for(auto &buffer : buffers){
        buffer.print();
        cout<<"\n";
    }
}

void divide(vector<File<T>> &tapes, vector<Buffer<T>> &buffers, ProgramInfo &settings, pair<int,int> fib){

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
}

int merge(vector<File<T>> &tapes, vector<Buffer<T>> &buffers, pair<int, int> fib, ProgramInfo &settings){
    int C = 0;
    int A = 1;
    int B = 2;

    int phaseCounter = 0;

    while(fib.second > 0){
        bool endOfFileB = false;
        for(int i=0;i<fib.second;i++){
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

    tapes[B].remove();
    tapes[C].remove();

    buffers[A].reset();
    if(A != 0){
        tapes[A].rename(tapes[0].getName());
    }

    return phaseCounter;

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

string getStringInput(string prompt){
    string filename;
    cout<<prompt<<": ";

    cin>>filename;

    return filename;
}

void inputDataIntoFile(string &filename){
    cout<<"Input your records (one record is two doubles seperated by a space), to stop writing, enter -1 as one of the values\n\n";
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

bool testOpenFile(string &filename){
    fstream test(filename);
    if(test.is_open()){
        test.close();
        return true;
    }
    cout<<"Wasn't able to open file "<<filename<<"\n";
    return false;
}

void printInputFile(string &filename){
    cout<<"Here is your file:\n";
    fstream f(filename);
    double a, r;
    while(f >> a >> r){
        cout<<a<<" "<<r<<"\n";
    }
    f.close();
}

ProgramInfo menu(){
    string filename = "";

    int b = getIntInput("Set your blocking factor (b)");

    bool existingFile = pickOption("Do you want to read data from an existing file?", {"Y", "N"}) == "Y";   
    if(existingFile){
        while(true){
            filename = getStringInput("Input the name of your file: ");
            if(testOpenFile(filename)){
                break;
            }
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
            inputDataIntoFile(filename);
        }
    }
    Record::asc = pickOption("Do you want to sort records ascending or descending?", {"1", "2"}) == "1"; 
    bool printing = pickOption("Do you want to print your files after every phase?", {"Y", "N"}) == "Y";
    bool testing = pickOption("Do you want to check if your file was sorted correctly?", {"Y", "N"}) == "Y";
    
    optional<Test<T>> test;
    if(testing){
        test = Test<T>(filename);
    }

    if(printing){
        printInputFile(filename);
    }

    return ProgramInfo(filename, printing, b, test);
}

SetupResult setup() {
    ProgramInfo settings = menu();

    string filename1 = settings.filename;
    string filename2 = "tape2.txt";
    string filename3 = "tape3.txt";

    vector<File<T>> tapes;
    tapes.emplace_back(filename1);
    tapes.emplace_back(filename2);
    tapes.emplace_back(filename3);
    
    vector<Buffer<T>> buffers;
    for(auto &tape : tapes){
        buffers.emplace_back(tape, settings.b);
    }

    return {move(settings), move(tapes), move(buffers)};
}

int sort(vector<File<T>> &tapes, vector<Buffer<T>> &buffers, ProgramInfo &settings){
    
    SORTBEGIN;
    int runs = countRuns(buffers[0]);
    buffers[0].reset();
    pair<int, int> fib = getFib(runs);
    int phases = 0;
    if(fib.second != 0){
        divide(tapes, buffers, settings, fib);
        phases = merge(tapes, buffers, fib, settings);
    }
    SORTEND;

    return phases;
}

void stats(ProgramInfo &settings, int phases){
    if(settings.test){
        bool ok = settings.test->check(settings.filename);
        cout << (ok ? "TEST PASSED\n" : "TEST FAILED\n");
    }
    cout<<"READS: "<<Buffer<T>::reads<<"\n";
    cout<<"WRITES: "<<Buffer<T>::writes<<"\n";
    cout<<"PHASES: "<<phases;
}

int main(){

    auto [settings, tapes, buffers] = setup();
    int phases = sort(tapes, buffers, settings);
    stats(settings, phases);

    return 0;
}