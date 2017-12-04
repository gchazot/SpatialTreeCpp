#pragma once
#include <algorithm>
#include <limits>
#include <list>
#include <memory>
#include <optional>
#include <set>
#include <vector>

using namespace std;

typedef double CoordinateType;
static const CoordinateType infinity = numeric_limits<CoordinateType>::max();

typedef vector<CoordinateType> Coordinates;

enum class DimensionType : Coordinates::size_type {};

DimensionType next(DimensionType dimension, DimensionType maxDimension);

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

	size_t GetId() const {
		return _id;
	}

	CoordinateType Component(DimensionType dimension) const {
		return _location[static_cast<Coordinates::size_type>(dimension)];
	}

	DimensionType Dimension() const {
		return DimensionType(_location.size());
	}

	CoordinateType Distance(const Point & other) const {
		CoordinateType total = 0.0f;
		for (size_t i = 0; i < _location.size(); ++i) {
			total += (_location[i] - other._location[i]) * (_location[i] - other._location[i]);
		}
		return sqrt(total);
	}

private:
	size_t _id;
	Coordinates _location;
};

class NearestSearch {
public:
	NearestSearch(const Point & point) :
		_point(point),
		_closest(nullopt) {
	}

	const Point & GetPoint() const {
		return _point;
	}
	CoordinateType GetClosestDistance() const {
		if (_closest.has_value()) {
			return _point.Distance(*_closest.value());
		}
		return infinity;
	}
	const Point * GetClosest() const {
		return _closest.value_or(nullptr);
	}

	void Update(const Point & point);

private:
	const Point & _point;
	optional<const Point *> _closest;
};

class SpatialBranch {
public:
	virtual void Add(Point && point) = 0;
	virtual bool MustSplit() const = 0;
	virtual void SearchNearest(NearestSearch & result) const = 0;

	virtual size_t Size() const = 0;
	virtual size_t NumLeaves() const = 0;
	virtual size_t Depth() const = 0;
	virtual size_t MaxItemsPerLeaf() const = 0;
};

class SpatialLeaf: public SpatialBranch {
public:
	SpatialLeaf(size_t maxItems):
		_maxItems(maxItems) {
	}

	virtual void Add(Point && point) override;
	virtual bool MustSplit() const override {
		return _points.size() > _maxItems;
	}
	virtual void SearchNearest(NearestSearch & result) const override;

	virtual size_t Size() const {
		return _points.size();
	}
	virtual size_t NumLeaves() const override {
		return 1;
	}
	virtual size_t Depth() const override {
		return 1;
	}
	virtual size_t MaxItemsPerLeaf() const override {
		return Size();
	}

	SpatialLeaf Split(DimensionType dimension, CoordinateType & splitValue);

private:
	size_t _maxItems;
	list<Point> _points;
};

class SpatialTree : public SpatialBranch {
public:
	SpatialTree(DimensionType splitDimension, CoordinateType splitValue);

	virtual void Add(Point && point) override;
	virtual bool MustSplit() const override {
		return false;
	}
	virtual void SearchNearest(NearestSearch & result) const override;

	virtual size_t Size() const {
		return _lb->Size() + _ub->Size();
	}
	virtual size_t NumLeaves() const override {
		return _lb->NumLeaves() + _ub->NumLeaves();
	}
	virtual size_t Depth() const override {
		return max(_lb->Depth(), _ub->Depth()) + 1;
	}
	virtual size_t MaxItemsPerLeaf() const override {
		return max(_lb->MaxItemsPerLeaf(), _ub->MaxItemsPerLeaf());
	}

private:
	unique_ptr<SpatialBranch> & WhichBranch(const Point & point);

	DimensionType _splitDimension;
	CoordinateType _splitValue;
	unique_ptr<SpatialBranch> _lb;
	unique_ptr<SpatialBranch> _ub;
	
};

