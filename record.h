#define ANGLE   0
#define RADIUS  1
struct Record{

    double angle;
    double radius;

    static int sortField;

    Record(){
        angle = 0;
        radius = 0;
    }

    Record(double a, double r){
        angle = a;
        radius = r;
    }

    bool operator<(const Record &other) const{
        if(sortField == ANGLE){
            return angle < other.angle;
        }
        return radius < other.radius;
    }

    friend std::ostream& operator<<(std::ostream &os, const Record &r){
        os << r.angle << " " << r.radius;
        return os;
    }

    friend std::istream& operator>>(std::istream& in, Record& r){
        in >> r.angle >> r.radius;
        return in;
    }

    bool operator==(const Record &other) const{
        if(sortField == ANGLE){
            return angle == other.angle;
        }
        return radius == other.radius;
    }
};
int Record::sortField;