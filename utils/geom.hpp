#include "image.hpp"
#include <iterator>
#include <vector>
#include <limits>
#include <cmath>

void fatal(const char*, ...);

static inline bool inrange(double min, double max, double x) {
 	return x >= min && x <= max;
}

struct Point {

	static Point cross(const Point &a, const Point &b) { return a.cross(b); }

	static double dot(const Point &a, const Point &b) { return a.dot(b); }

	static Point sub(const Point &a, const Point &b) { return a.minus(b); }

	static double distance(const Point &a, const Point &b) {
		Point diff = b.minus(a);
		return sqrt(dot(diff, diff));
	}

	static Point inf(void) {
		return Point(std::numeric_limits<double>::infinity(),
			std::numeric_limits<double>::infinity());
	}

	static Point neginf(void) {
		return Point(-std::numeric_limits<double>::infinity(),
			-std::numeric_limits<double>::infinity());
	}

	// Get the angle off of the positive x axis
	// between 0 and 2π.
	static double angle(const Point &a) {
		double angle = atan2(a.y, a.x);
		if (angle < 0)
			return 2 * M_PI + angle;
		return angle;
	}

	Point(void) { }

	Point(double _x, double _y) : x(_x), y(_y), z(0.0) { }

	Point(double _x, double _y, double _z) : x(_x), y(_y), z(_z) { }

	void draw(Image &img, Color c = Image::black, double r = 1) const {
		img.add(new Image::Circle(x, y, r,  c, 0.1));
	}

	Point cross(const Point &b) const {
		return Point(
			y * b.z - z * b.y,
			z * b.x - x * b.z,
			x * b.y - y * b.x);
	}

	double dot(const Point &b) const {
		return x * b.x + y * b.y + z * b.z;
	}

	Point minus(const Point &b) const {
		return Point(x - b.x, y - b.y, z - b.z);
	}

	double x, y, z;
};

struct Line {
	Line(void) { }

	Line(Point p0, Point p1) {
		double dx = p1.x - p0.x;
		double dy = p1.y - p0.y;
		m = dy / dx;
		b = p0.y - m * p0.x;
	}

	// If they are parallel then (∞,∞), if they are
	// the same line then (FP_NAN, FP_NAN)
	Point isect(const Line &l) const {
		double x = (l.b - b) / (m - l.m);
		return Point(x, m * x + b);
	}

	bool isabove(const Point &p) const { return (m * p.x + b) < p.y; }

	double m, b;
};

struct LineSeg : public Line {
	LineSeg(void) { }

	LineSeg(Point _p0, Point _p1) : Line(_p0, _p1), p0(_p0), p1(_p1) {
		if (p1.x < p0.x) {
			mins.x = p1.x;
			maxes.x = p0.x;
		} else {
			mins.x = p0.x;
			maxes.x = p1.x;
		}
		if (p1.y < p0.y) {
			mins.y = p1.y;
			maxes.y = p0.y;
		} else {
			mins.y = p0.y;
			maxes.y = p1.y;
		}
	}

	void draw(Image &img, Color c = Image::black, double w = 1) const {
		img.add(new Image::Line(p0.x, p0.y, p1.x, p1.y, w, c));
	}

	Point midpt(void) const {
		return Point((p0.x + p1.x) / 2, (p0.y + p1.y) / 2);
	}

	Point along(double dist) const {
		double theta = Point::angle(p1.minus(p0));
		return Point(p0.x + dist * cos(theta), p0.y + dist * sin(theta));
	}

	double length(void) const {
		return Point::distance(p0, p1);
	}

	bool contains(const Point &p) const {
		return inrange(mins.x, maxes.x, p.x) && inrange(mins.y, maxes.y, p.y);
	}

	Point isect(const LineSeg &l) const {
		if (isvertical() || l.isvertical())
			return vertisect(*this, l);

		Point p = Line::isect(l);
		if (!contains(p) || !l.contains(p))
			return Point::inf();
		return p;
	}

	bool isvertical(void) const {
		return p0.x == p1.x;
	}

	Point p0, p1;
	Point mins, maxes;

private:

	// Deal with vertical line segments
	static Point vertisect(const LineSeg &a, const LineSeg &b) {
		if (a.isvertical() && b.isvertical())
			return Point::inf();

		const LineSeg *v = &a, *l = &b;
		if (b.isvertical()) {
			v = &b;
			l = &a;
		}
		Point p(v->p0.x, l->m * v->p0.x + l->b);
		if (!a.contains(p) || !b.contains(p))
			return Point::inf();

		return p;
	}
};

struct Bbox {
	Bbox(void) : min(Point::inf()), max(Point::neginf()) { }

	Bbox(const Point &_min, const Point &_max) : min(_min), max(_max) { }

	// Get the bounding box for the set of points
	Bbox(std::vector<Point> &pts) {
		min = Point::inf();
		max = Point::neginf();

		for (unsigned int i = 0; i < pts.size(); i++) {
			Point &p = pts[i];
			if (p.x > max.x)
				max.x = p.x;
			if (p.x < min.x)
				min.x = p.x;
			if (p.y > max.y)
				max.y = p.y;
			if (p.y < min.y)
				min.y = p.y;
		}
	}

	void draw(Image&, Color c = Image::black, double lwidth = 1) const;

	bool contains(const Point &p) const {
		return p.x > min.x && p.y > min.y && p.x < max.x && p.y < max.y;
	}

	bool isect(const Bbox &b) const {
		return (inrange(min.x, max.x, b.min.x) || inrange(min.x, max.x, b.max.x))
			&& (inrange(min.y, max.y, b.min.y) || inrange(min.y, max.y, b.max.y));
	}

	Point min, max;
};

struct Polygon {

	static Polygon random(unsigned int n, double xc, double yc, double r);

	// vertices are given clock-wise around the polygon.
	Polygon(std::vector<Point> &pts) : verts(pts), bbox(pts) {
		if (pts.size() < 3)
			fatal("A polygon needs at least 3 points\n");
		initsides(verts);
	}

	// If the lwidth is <0 then the polygon is filled.
	void draw(Image&, Color c = Image::black, double lwidth = 1) const;

	bool contains(const Point&) const;

	void isects(const LineSeg&, std::vector<Point>&) const;
 
	Point minisect(const LineSeg&) const;

	bool hits(const LineSeg &) const;

	void reflexes(std::vector<Point>&) const;

	std::vector<Point> verts;
	std::vector<LineSeg> sides;
	Bbox bbox;

private:
	void initsides(std::vector<Point>&);
};