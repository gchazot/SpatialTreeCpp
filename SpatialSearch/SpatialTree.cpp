#include "SpatialTree.h"

#include <algorithm>

inline void SpatialLeaf::Add(Point && point) {
	_points.push_back(point);
}

SpatialLeaf SpatialLeaf::split(DimensionType dimension)
{
	_points.sort(DimensionComparator(dimension));
	
	auto middle = _points.begin();
	for (size_t i = 0; i < _points.size() / 2; ++i) {
		++middle;
	}

	Bounds newBounds(_bounds.split(dimension));
	
	SpatialLeaf newLeaf(move(newBounds), _maxItems);
	newLeaf._points.splice(newLeaf._points.end(), _points, middle, _points.end());
	return newLeaf;
}

void SpatialTree::Add(Point && point)
{
	if (point.component(_splitDimension) < _splitValue) {
		_lb->Add(move(point));
	} else {
		_ub->Add(move(point));
	}
}

Bounds Bounds::split(DimensionType dimension)
{
	const auto dimIndex = static_cast<underlying_type<DimensionType>::type>(dimension);
	const CoordinateType splitValue = (_mins[dimIndex] + _maxs[dimIndex]) / 2;

	_mins[dimIndex] = splitValue;

	Bounds newBounds(*this);
	newBounds._maxs[dimIndex] = splitValue;

	return newBounds;
}
