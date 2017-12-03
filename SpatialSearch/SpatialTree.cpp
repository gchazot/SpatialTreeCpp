#include "SpatialTree.h"

#include <algorithm>

DimensionType next(DimensionType dimension, DimensionType maxDimension)
{
	auto dim = static_cast<underlying_type<DimensionType>::type>(dimension);
	auto maxDim = static_cast<underlying_type<DimensionType>::type>(maxDimension);
	return DimensionType((dim + 1) % maxDim);
}

Bounds Bounds::Split(DimensionType dimension, CoordinateType splitValue)
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

SpatialLeaf SpatialLeaf::Split(DimensionType dimension, CoordinateType & splitValue)
{
	_points.sort(DimensionComparator(dimension));
	
	auto middle = _points.begin();
	for (size_t i = 0; i < _points.size() / 2; ++i) {
		++middle;
	}

	splitValue = middle->Component(dimension);
	Bounds newBounds(_bounds.Split(dimension, splitValue));
	
	SpatialLeaf newLeaf(move(newBounds), _maxItems);
	newLeaf._points.splice(newLeaf._points.end(), _points, middle, _points.end());
	return newLeaf;
}

SpatialTree::SpatialTree(DimensionType splitDimension, CoordinateType splitValue) :
	_splitDimension(splitDimension),
	_splitValue(splitValue) {
	Bounds highBounds({ -1.0, -1.0, -1.0 }, { 1.0, 1.0, 1.0 });
	Bounds lowBounds(move(highBounds.Split(_splitDimension, _splitValue)));
	_lb.reset(new SpatialLeaf(move(lowBounds), 10));
	_ub.reset(new SpatialLeaf(move(highBounds), 10));
}

void SpatialTree::Add(Point && point)
{
	if (point.Component(_splitDimension) < _splitValue) {
		_lb->Add(move(point));

		if (_lb->MustSplit()) {
			DimensionType nextDimension = next(_splitDimension, point.Dimension());
			auto leaf = dynamic_cast<SpatialLeaf*>(_lb.get());

			CoordinateType splitValue;
			auto newLeaf = new SpatialLeaf(leaf->Split(nextDimension, splitValue));

			auto newTree = new SpatialTree(nextDimension, splitValue);
			newTree->_lb = move(_lb);
			newTree->_ub.reset(newLeaf);
			_lb.reset(newTree);
		}
	} else {
		_ub->Add(move(point));

		if (_ub->MustSplit()) {
			DimensionType nextDimension = next(_splitDimension, point.Dimension());
			auto leaf = dynamic_cast<SpatialLeaf*>(_ub.get());

			CoordinateType splitValue;
			auto newLeaf = new SpatialLeaf(leaf->Split(nextDimension, splitValue));

			auto newTree = new SpatialTree(nextDimension, splitValue);
			newTree->_lb = move(_ub);
			newTree->_ub.reset(newLeaf);
			_ub.reset(newTree);
		}
	}
}
