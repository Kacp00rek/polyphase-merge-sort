#include <numbers>
#include <iostream>

using namespace std;

enum SortingMode {ASC, DESC};

struct Record{

    double angle;
    double radius;

    static SortingMode mode;

    Record(){
        angle = 0;
        radius = 0;
    }

    Record(double a, double r){
        angle = a;
        radius = r;
    }

    double getField() const {
        return numbers::pi * radius * radius * angle / 360;
    }

    bool operator<(const Record &other) const {
        if(mode == ASC){
            return getField() < other.getField();
        }
        return getField() > other.getField();

    }

    friend ostream& operator<<(ostream &os, const Record &r){
        os << r.angle << " " << r.radius;
        return os;
    }

    friend istream& operator>>(istream& in, Record& r){
        double ang, rad;
        if(in >> ang >> rad){
            if(ang > 0 && ang <= 360 && rad > 0){
                r.angle = ang;
                r.radius = rad;
            }
            else{
                in.setstate(ios_base::failbit);
            }
        }
        return in;
    }

    static string inputFormat(){
        return "<angle> <radius>  (e.g. 123.5  42.8)";
    }


    void print(){
        cout << *this << " [" << getField() << "]";
    }

};
SortingMode Record::mode;