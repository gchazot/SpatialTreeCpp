#include "SpatialTree.h"

#include <algorithm>

DimensionType next(DimensionType dimension, DimensionType maxDimension)
{
	auto dim = static_cast<underlying_type<DimensionType>::type>(dimension);
	auto maxDim = static_cast<underlying_type<DimensionType>::type>(maxDimension);
	return DimensionType((dim + 1) % maxDim);
}

void NearestSearch::Update(const Point & point) {
	if (_point.GetId() == point.GetId()) {
		return;
	}

	const double distance = _point.Distance(point);
	if (!_closest.has_value() || distance < _point.Distance(*_closest.value())) {
		_closest = &point;
	}
}

void SpatialLeaf::Add(Point && point) {
	_points.push_back(point);
}

void SpatialLeaf::SearchNearest(NearestSearch & result) const
{
	for (const auto & point : _points) {
		result.Update(point);
	}
}

SpatialLeaf SpatialLeaf::Split(DimensionType dimension, CoordinateType & splitValue)
{
	_points.sort(DimensionComparator(dimension));
	
	auto middle = _points.begin();
	for (size_t i = 0; i < _points.size() / 2; ++i) {
		++middle;
	}
	while (middle != _points.begin() && prev(middle)->Component(dimension) == middle->Component(dimension)) {
		--middle;
	}

	splitValue = middle->Component(dimension);
	
	SpatialLeaf newLeaf(_maxItems);
	newLeaf._points.splice(newLeaf._points.end(), _points, middle, _points.end());
	return newLeaf;
}

SpatialTree::SpatialTree(DimensionType splitDimension, CoordinateType splitValue) :
	_splitDimension(splitDimension),
	_splitValue(splitValue) {
	_lb.reset(new SpatialLeaf(10));
	_ub.reset(new SpatialLeaf(10));
}

void SpatialTree::Add(Point && point)
{
	unique_ptr<SpatialBranch> & branch = WhichBranch(point);
	branch->Add(move(point));

	if (branch->MustSplit()) {
		DimensionType nextDimension = next(_splitDimension, point.Dimension());
		auto leaf = dynamic_cast<SpatialLeaf*>(branch.get());

		CoordinateType splitValue;
		auto newLeaf = new SpatialLeaf(leaf->Split(nextDimension, splitValue));

		auto newTree = new SpatialTree(nextDimension, splitValue);
		newTree->_lb = move(branch);
		newTree->_ub.reset(newLeaf);
		branch.reset(newTree);
	}
}

void SpatialTree::SearchNearest(NearestSearch & result) const
{
	const CoordinateType targetCoord = result.GetPoint().Component(_splitDimension);
	if (targetCoord < _splitValue) {
		_lb->SearchNearest(result);
		if (targetCoord + result.GetClosestDistance() >= _splitValue) {
			_ub->SearchNearest(result);
		}
	} else {
		_ub->SearchNearest(result);
		if (targetCoord - result.GetClosestDistance() < _splitValue) {
			_lb->SearchNearest(result);
		}
	}
}

unique_ptr<SpatialBranch> & SpatialTree::WhichBranch(const Point & point)
{
	if (point.Component(_splitDimension) < _splitValue) {
		return _lb;
	} else {
		return _ub;
	}
}
