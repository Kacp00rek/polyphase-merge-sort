#include <iostream>
#include <vector>
#include <random>
#include <limits>
#include "file.h"
#include "record.h"
#include "buffer.h"
#include "test.h"

using namespace std;
using RecordType = Record;

struct ProgramInfo{
    string filename;
    bool printing;
    optional<Test<RecordType>> test;
    int b;
    ProgramInfo(string f, bool p, int blockingFactor, optional<Test<RecordType>> t){
        filename = f;
        printing = p;
        b = blockingFactor;
        test = t;
    }
};

struct SetupResult{
    ProgramInfo settings;
    vector<File<RecordType>> tapes;
    vector<Buffer<RecordType>> buffers;
};

pair<int, int> getFib(int n){
    pair<int,int> fib = {1, 0};

    while(fib.first + fib.second < n){
        fib = {fib.first + fib.second, fib.first};
    }

    return fib;
}

int countRuns(Buffer<RecordType> &buff){
    auto temp = buff.next();
    if(!temp){
        return 0;
    }

    RecordType previous = *temp;
    int counter = 1;

    while(auto temp = buff.next()){
        RecordType record = *temp;
        if(record < previous){
            counter++;  
        }
        previous = record;
    }
    return counter;
}

void printFiles(vector<Buffer<RecordType>> &buffers){
    for(auto &buffer : buffers){
        buffer.print();
        cout << "\n";
    }
}

void divide(vector<File<RecordType>> &tapes, vector<Buffer<RecordType>> &buffers, ProgramInfo &settings, pair<int,int> fib){

    int runCounter = 0;
    optional<RecordType> previous;
    int currTape = 1;

    while(auto temp = buffers[0].next()){
        RecordType record = *temp;
        if(previous && record < *previous){
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
    for(auto &buf : buffers){
        buf.reset();
    }

    if(settings.printing){
        cout << "INITIAL DISTRIBUTION:\n\n";
        printFiles(buffers);
        cout << "\n";
    }
}

void logPhase(int phase, vector<Buffer<RecordType>>& buffers) {
    cout << "PHASE " << phase << ":\n";
    printFiles(buffers);
    cout << "\n";
}

bool writeAndCheckRunEnd(optional<RecordType> &record, Buffer<RecordType> &src, Buffer<RecordType> &dest){
    dest.write(*record);
    RecordType prev = *record;
    record = src.next();
    if(!record || *record < prev){
        if(record){
            src.reRead();
        }
        return true;
    }
    return false;
}

void mergeRuns(Buffer<RecordType>& A, Buffer<RecordType>& B, Buffer<RecordType>& dest){

    bool endA = false;
    bool endB = false;
    optional<RecordType> recA = A.next();
    optional<RecordType> recB = B.next();
    if(!recB){
        endB = true;
    }

    while(!endB || !endA){
        if(!endA && (endB || (*recA < *recB))){
            endA = writeAndCheckRunEnd(recA, A, dest);
        }
        else{
            endB = writeAndCheckRunEnd(recB, B, dest);
        }
    }

}

void finalizePhase(int &A, int &B, int &C, vector<Buffer<RecordType>> &buffers, vector<File<RecordType>> &tapes, pair<int, int> &fib, int &phaseCounter){
    buffers[C].flush();
    buffers[C].reset();
    buffers[B].reset();
    tapes[B].clear();

    A = (A + 2) % 3;
    B = (B + 2) % 3;
    C = (C + 2) % 3;

    fib = {fib.second, fib.first - fib.second};
    phaseCounter++;
}

int merge(vector<File<RecordType>> &tapes, vector<Buffer<RecordType>> &buffers, pair<int, int> fib, ProgramInfo &settings){
    int C = 0, A = 1, B = 2;
    int phaseCounter = 0;

    while(fib.second > 0){
        for(int i=0;i<fib.second;i++){
            mergeRuns(buffers[A], buffers[B], buffers[C]);
        }
        
        finalizePhase(A, B, C, buffers, tapes, fib, phaseCounter);

        if(settings.printing){
            logPhase(phaseCounter, buffers);
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
    uniform_real_distribution<double> radius(0, 100);

    ofstream file(filename);

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
        cout << prompt << " "<< op;
        cin>>answer;
        if(find(options.begin(), options.end(), answer) != options.end()){
            return answer;
        }
    }
}

int getIntInput(string prompt){
    int value;
    while(true){
        cout << prompt << ": ";
        if(cin >> value && value > 0){
            return value;
        }
        cout << "Invalid input\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

string getStringInput(string prompt){
    string filename;
    cout << prompt << ": ";

    cin >> filename;

    return filename;
}

void inputDataIntoFile(string &filename){
    string endKeyword = "end";
    cout << "Enter your records (one per line), to stop enter '" << endKeyword << "'\n";
    ofstream file(filename);       
    string line;
    getline(cin, line);
    while(true){
        cout<<">";
        if(!getline(cin, line) || line == endKeyword){
            break;
        }
        stringstream ss(line);
        RecordType record;
        if(ss >> record){
            file << record << "\n";
        }
        else{
            cout << "Invalid input\n";
        }
    }
    file.close();
}

bool testOpenFile(string &filename){
    fstream test(filename);
    if(test.is_open()){
        test.close();
        return true;
    }
    cout << "Wasn't able to open file " << filename << "\n";
    return false;
}

string getExistingFilename() {
    while (true) {
        string filename = getStringInput("Input the name of your file");
        if(testOpenFile(filename)){
            return filename;
        }
    }
}

void createInputFile(string &filename){
    bool generate = pickOption("Do you want to generate random data or type the records yourself?", {"1", "2"}) == "1";

    if(generate){
        int N = getIntInput("Number of records to generate");
        generateData(N, filename);
    }
    else{
        inputDataIntoFile(filename);
    }
}

void printInputFile(string &filename){
    cout<<"\nHere is your file:\n";
    fstream f(filename);
    RecordType record;
    while(f >> record){
        cout << record << "\n";
    }
    f.close();
}

ProgramInfo menu(){
    string filename;
    int b = getIntInput("Set your blocking factor (b)");

    bool existingFile = pickOption("Do you want to read data from an existing file?", {"y", "n"}) == "y";   
    if(existingFile){
        filename = getExistingFilename();
    }
    else{
        filename = "input.txt";
        createInputFile(filename);
    }
    bool ascending = pickOption("Do you want to sort records ascending or descending?", {"1", "2"}) == "1"; 
    RecordType::mode = ascending ? ASC : DESC;
    bool printing = pickOption("Do you want to print your files after every phase?", {"y", "n"}) == "y"; 
    bool testing = pickOption("Do you want to check if your file was sorted correctly?", {"y", "n"}) == "y"; 
    
    optional<Test<RecordType>> test;
    if(testing){
        test = Test<RecordType>(filename);
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

    vector<File<RecordType>> tapes;
    tapes.emplace_back(filename1);
    tapes.emplace_back(filename2);
    tapes.emplace_back(filename3);
    
    vector<Buffer<RecordType>> buffers;
    for(auto &tape : tapes){
        buffers.emplace_back(tape, settings.b);
    }

    return {move(settings), move(tapes), move(buffers)};
}

int sort(vector<File<RecordType>> &tapes, vector<Buffer<RecordType>> &buffers, ProgramInfo &settings){
    
    cout << "\n--- SORTING BEGIN ---\n";
    int runs = countRuns(buffers[0]);
    buffers[0].reset();
    pair<int, int> fib = getFib(runs);
    int phases = 0;
    if(fib.second != 0){
        divide(tapes, buffers, settings, fib);
        phases = merge(tapes, buffers, fib, settings);
    }
    cout << "\n--- SORTING COMPLETE ---\n";

    return phases;
}

void stats(ProgramInfo &settings, int phases){
    if(settings.test){
        bool ok = settings.test->check(settings.filename);
        cout << (ok ? "TEST PASSED\n" : "TEST FAILED\n");
    }
    cout << "READS: " <<Buffer<RecordType>::reads<< "\n";
    cout << "WRITES: " <<Buffer<RecordType>::writes<<"\n";
    cout << "PHASES: " <<phases;
}

int main(){

    auto [settings, tapes, buffers] = setup();
    int phases = sort(tapes, buffers, settings);
    stats(settings, phases);

    return 0;
}