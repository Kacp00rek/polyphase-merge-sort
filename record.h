struct Record{

    double angle;
    double radius;

    Record(){
        angle = 0;
        radius = 0;
    }

    Record(double a, double r){
        angle = a;
        radius = r;
    }

    bool operator<(const Record &other) const{
        return angle < other.angle;
    }

    friend std::ostream& operator<<(std::ostream &os, const Record &r){
        os << r.angle << " " << r.radius;
        return os;
    }

    friend std::istream& operator>>(std::istream& in, Record& r){
        in >> r.angle >> r.radius;
        return in;
    }

};