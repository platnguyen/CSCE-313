#include <iostream>
#include <cstring>
struct Point {
    int x, y;

    Point () : x(), y() {}
    Point (int _x, int _y) : x(_x), y(_y) {}
};

class Shape {
private:
    int vertices;
    Point** points;
public:
    Shape (int _vertices) {
        vertices = _vertices;
        points = new Point*[vertices+1];
        for (int i = 0; i < vertices; ++i) {
	    points[i] = new Point();
	}
    }

    ~Shape () {
        for (int i = 0; i < vertices; ++i) {
	    delete points[i];
	}
	delete[] points;
    }

    void addPoints(Point* pts) {
        for (int i = 0; i < vertices; i++) {
            memcpy(points[i], &pts[i], sizeof(Point));
        }
    }

    double area () {
        int temp = 0;
	//loop through amount of vertices
        for (int i = 0; i < vertices; i++) {
	    /*lhs found by taking the x from each vertex and multiplying it by
	    the next vertex, looping back to the very first one when x is at the end
	    to not overflow.*/
            int lhs = points[i]->x * points[(i+1)%vertices]->y;
	    //does the same thing as lhs, just vice versa
            int rhs = (*points[(i+1)%vertices]).x * (*points[i]).y;
            temp += (lhs - rhs);
        }
        double calc_area = abs(temp)/2.0;
        return calc_area;
    }
};

int main () {
    //define all the triangle points in different ways
    Point tri1(0,0);
    Point tri2 = {1,2};
    Point tri3 = Point(2,0);
    // adding points to tri
    Point triPts[3] = {tri1, tri2, tri3};
    Shape* tri = new Shape(3);
    tri->addPoints(triPts);
    //define the points of the quadrilateral
    Point quad1 = {0,0};
    Point quad2 = {0,2};
    Point quad3 = {2,2};
    Point quad4 = {2,0};
    // adding points to quad
    Point quadPts[4] = {quad1, quad2, quad3, quad4};
    Shape* quad = new Shape(4);
    quad->addPoints(quadPts);
    //print our calculated area and cleanup the objects tri and quad
    std::cout << "Area of triangle is: " << tri->area() << std::endl;
    std::cout << "Area o fquadrilateral is: " << quad->area() << std::endl;
    delete tri;
    delete quad; 

}
