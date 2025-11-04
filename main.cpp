#include <iostream>
#include <vector>
#include <random>
#include "file.h"
#include "record.h"
#include "buffer.h"
#include "test.h"

using namespace std;

template <typename T>
struct ProgramInfo{
    string filename;
    bool printing;
    bool testing;
    int b;
    Test<T> test;
    ProgramInfo(string f, bool p, int blockingFactor){
        filename = f;
        printing = p;
        b = blockingFactor;
        testing = true;
        if(testing){
            test = Test<T>(filename);
        }
    }

};

pair<bool, int> convertStringToInt(string s){
    pair<bool, int> answer = {true, 0};
    if(s.length() == 0 || s[0] == '0'){
        answer.first = false;
        return answer;
    }
    for(int i=0;i<s.length();i++){
        if(!isdigit(s[i])){
            answer.first = false;
            break;
        }
        answer.second = answer.second * 10 + (int)(s[i] - '0');
    }
    return answer;
}

template <typename T>
void copyFile(Buffer<T> &buffer1, Buffer<T> &buffer2){
    while(auto record = buffer2.next()){
        buffer1.write(*record);
    }
    buffer1.flush();
}

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
pair<int ,int> divide(vector<File<T>> &tapes, vector<Buffer<T>> &buffers, ProgramInfo<T> &settings){
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
        cout<<"\nPHASE 1:\n\n";
        printFiles(buffers);
        cout<<"\n";
    }

    //cout<<runs<<" "<<fib.first<<" "<<fib.second<<"\n";

    return fib;
}

template <typename T>
void merge(vector<File<T>> &tapes, vector<Buffer<T>> &buffers, pair<int, int> fib, ProgramInfo<T> &settings){
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
        if(settings.printing){
            cout<<"\nPHASE "<<++phaseCounter<<":\n\n";
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

    bool verdict = settings.test.check(tapes[0].getName());

    if(verdict){
        cout<<"TEST PASSED\n";
    }
    else{
        cout<<"TEST FAILED\n";
    }

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

template <typename T>
ProgramInfo<T> menu(){
    string answer = "", filename = "";

    int b;
    while(true){
        cout<<"What is your desired value for the blocking factor (b)?\n";
        string temp;
        cin>>temp;
        auto ans = convertStringToInt(temp);
        if(ans.first){
            b = ans.second;
            break;
        }
    }

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

    answer = "";
    while(answer != "1" && answer != "2"){
        cout<<"Do you want to sort records by angle (1) or by radius (2)?\n";
        cin>>answer;
        if(answer == "1"){
            Record::sortField = ANGLE;
        }
        else if(answer == "2"){
            Record::sortField = RADIUS;
        }
    }    

    bool printing;
    string p = "";
    while(p != "N" && p != "Y"){
        cout<<"Do you want to print your files after every phase? (Y/N)\n";
        cin>>p;
        if(p == "N"){
            printing = false;
        }
        else if(p == "Y"){
            printing = true;
        }
    }


    if(printing){
        cout<<"Here is your file:\n";
        fstream f(filename);
        double a, r;
        while(f >> a >> r){
            cout<<a<<" "<<r<<"\n";
        }
        f.close();
    }

    return ProgramInfo<T>(filename, printing, b);
}

int main(){

    ProgramInfo<Record> settings = menu<Record>();

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

    pair<int, int> fibDivision = divide(tapes, buffers, settings);
    merge(tapes, buffers, fibDivision, settings);

    cout<<"READS: "<<Buffer<Record>::reads<<"\n";
    cout<<"WRITES: "<<Buffer<Record>::writes;


    return 0;
}