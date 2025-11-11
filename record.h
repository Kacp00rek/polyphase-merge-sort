#include <numbers>
#include <iostream>

#define ANGLE   0
#define RADIUS  1
struct Record{

    double angle;
    double radius;

    static bool asc;

    Record(){
        angle = 0;
        radius = 0;
    }

    Record(double a, double r){
        angle = a;
        radius = r;
    }

    double getField() const {
        return std::numbers::pi * radius * radius * angle / 360;
    }

    bool operator<(const Record &other) const {
        if(asc){
            return getField() < other.getField();
        }
        return getField() > other.getField();

    }

    friend std::ostream& operator<<(std::ostream &os, const Record &r){
        os << r.angle << " " << r.radius;
        return os;
    }

    friend std::istream& operator>>(std::istream& in, Record& r){
        in >> r.angle >> r.radius;
        return in;
    }

    bool operator==(const Record &other) const {
        return getField() == other.getField();
    }
};
bool Record::asc;