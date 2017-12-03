#include "SpatialTree.h"

#include <algorithm>

Bounds Bounds::split(DimensionType dimension, CoordinateType splitValue)
{
	const auto dimIndex = static_cast<underlying_type<DimensionType>::type>(dimension);

	_mins[dimIndex] = splitValue;

	Bounds newBounds(*this);
	newBounds._maxs[dimIndex] = splitValue;

	return newBounds;
}

void SpatialLeaf::Add(Point && point) {
	_points.push_back(point);
}

SpatialLeaf SpatialLeaf::split(DimensionType dimension)
{
	_points.sort(DimensionComparator(dimension));
	
	auto middle = _points.begin();
	for (size_t i = 0; i < _points.size() / 2; ++i) {
		++middle;
	}

	Bounds newBounds(_bounds.split(dimension, middle->component(dimension)));
	
	SpatialLeaf newLeaf(move(newBounds), _maxItems);
	newLeaf._points.splice(newLeaf._points.end(), _points, middle, _points.end());
	return newLeaf;
}

SpatialTree::SpatialTree(DimensionType splitDimension, CoordinateType splitValue) :
	_splitDimension(splitDimension),
	_splitValue(splitValue) {
	Bounds highBounds({ -1.0, -1.0, -1.0 }, { 1.0, 1.0, 1.0 });
	Bounds lowBounds(move(highBounds.split(_splitDimension, _splitValue)));
	_lb.reset(new SpatialLeaf(move(lowBounds), 10));
	_ub.reset(new SpatialLeaf(move(highBounds), 10));
}

void SpatialTree::Add(Point && point)
{
	if (point.component(_splitDimension) < _splitValue) {
		_lb->Add(move(point));
	} else {
		_ub->Add(move(point));
	}
}
