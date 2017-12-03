#pragma once
#include <limits>
#include <list>
#include <memory>
#include <set>
#include <vector>

using namespace std;

typedef double CoordinateType;
static const CoordinateType infinity = numeric_limits<CoordinateType>::max();

typedef vector<CoordinateType> Coordinates;

enum class DimensionType : Coordinates::size_type {};

class Point {
public:
	Point(size_t id, const Coordinates & location):
		_id(id),
		_location(location) {
	}

	Point(size_t id, Coordinates && location) :
		_id(id),
		_location(move(location)) {
	}

	CoordinateType component(DimensionType dimension) const {
		return _location[static_cast<Coordinates::size_type>(dimension)];
	}

private:
	size_t _id;
	Coordinates _location;
};

class DimensionComparator {
public:
	explicit DimensionComparator(DimensionType dimension) :
		_dim(dimension) {
	}

	DimensionComparator(const DimensionComparator &) = default;
	DimensionComparator(DimensionComparator &&) = default;


	bool operator()(const Point & p1, const Point & p2) const {
		return p1.component(_dim) < p2.component(_dim);
	}
private:
	const DimensionType _dim;
};

class Bounds {
public:
	Bounds(const Coordinates & mins, const Coordinates & maxs) :
		_mins(mins),
		_maxs(maxs) {
	}

	Bounds(Coordinates && mins, Coordinates && maxs) :
		_mins(move(mins)),
		_maxs(move(maxs)) {
	}

	Bounds(const Bounds & bounds) = default;
	Bounds(Bounds && bounds) = default;

	Bounds split(DimensionType dimension, CoordinateType splitValue);

private:
	Coordinates _mins;
	Coordinates _maxs;
};

class SpatialBranch {
public:
	virtual void Add(Point && point) = 0;
};

class SpatialLeaf: public SpatialBranch {
public:
	SpatialLeaf(Bounds && bounds, size_t maxItems):
		_bounds(move(bounds)),
		_maxItems(maxItems) {
	}

	virtual void Add(Point && point) override;

	SpatialLeaf split(DimensionType dimension);

private:
	Bounds _bounds;
	size_t _maxItems;
	list<Point> _points;
};

class SpatialTree : public SpatialBranch {
public:
	SpatialTree(DimensionType splitDimension, CoordinateType splitValue);

	virtual void Add(Point && point) override;
private:
	DimensionType _splitDimension;
	CoordinateType _splitValue;
	unique_ptr<SpatialBranch> _lb;
	unique_ptr<SpatialBranch> _ub;
	
};

