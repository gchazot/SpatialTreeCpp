#include <iomanip>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <cmath>
#include <math.h>
#include <limits>

#include "SpatialTree.h"

using namespace std;

#define debug(message) cerr << message << endl
#define print(message) cout << message << endl


static const double PI = 3.14159265358979323846;
static const double earthRadiusKm = 6371.0;

double deg2rad(double deg) {
	return (deg * PI / 180);
}

double rad2deg(double rad) {
	return (rad * 180 / PI);
}

struct Flight {
	Flight(size_t index, string callSign, double latitude, double longitude) :
		index(index), callSign(move(callSign)), latitude(latitude), longitude(longitude) {}

	size_t index;
	string callSign;
	double latitude;
	double longitude;

	double distance(const Flight & other) const {
		// Credits to https://stackoverflow.com/questions/10198985/calculating-the-distance-between-2-latitudes-and-longitudes-that-are-saved-in-a
		double lat1r, lon1r, lat2r, lon2r, u, v;
		lat1r = deg2rad(latitude);
		lon1r = deg2rad(longitude);
		lat2r = deg2rad(other.latitude);
		lon2r = deg2rad(other.longitude);
		u = sin((lat2r - lat1r) / 2);
		v = sin((lon2r - lon1r) / 2);
		return 2.0 * earthRadiusKm * asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v));
	}
};

Flight parse_line(const string line) {
	const auto comma1 = find(line.begin(), line.end(), ',');
	if (comma1 == line.end()) {
		throw runtime_error("Can't parse call sign: " + line);
	}

	const auto comma2 = find(next(comma1), line.end(), ',');
	if (comma2 == line.end()) {
		throw runtime_error("Can't parse latitude: " + line);
	}

	const auto callSign = line.substr(0, comma1 - line.begin());
	const auto latitude = line.substr(comma1 - line.begin() + 1, comma2 - comma1 - 1);
	const auto longitude = line.substr(comma2 - line.begin() + 1, line.end() - comma2);
	
	try {
		double lat = stod(latitude);
		double lon = stod(longitude);
		return Flight(0, move(callSign), lat, lon);
	}
	catch (invalid_argument e) {
		throw runtime_error("Unable to parse " + line);
	}
}

vector<Flight> parseFile(string fileName) {
	vector<Flight> flights;

	ifstream input_file(fileName);
	if (!input_file.is_open()) {
		return flights;
	}

	string line;
	for (size_t n = 0; getline(input_file, line);) {
		try {
			Flight newFlight = parse_line(line);
			newFlight.index = n;
			++n;

			flights.push_back(move(newFlight));
		}
		catch (runtime_error e) {
			debug(e.what());
			continue;
		}
	}

	return flights;
}

void print_result(const Flight & flight1, const Flight & flight2) {
	cout << setfill(' ');
	cout << setw(8) << left << flight1.callSign << " ";
	cout << setw(8) << left << flight2.callSign << " ";
	cout << setw(8) << right << fixed << setprecision(2) << flight1.distance(flight2);
	cout << endl;
}

void SolveWithForForLoop(vector<Flight> flights) {
	for (auto flight1 = flights.begin(); flight1 != flights.end(); ++flight1) {
		Flight * closest = nullptr;
		double min_distance = numeric_limits<double>::max();

		for (auto flight2 = flights.begin(); flight2 != flights.end(); ++flight2) {
			if (flight1 == flight2) {
				continue;
			}

			const auto distance = flight1->distance(*flight2);
			if (distance < min_distance) {
				closest = &*flight2;
				min_distance = distance;
			}
		}

		print_result(*flight1, *closest);
	}
}

int main()
{
	vector< Flight > flights = parseFile("data/20170901_080005.csv");

	if (flights.size() < 2) {
		// No reason to calculate distances
		return 0;
	}

	SolveWithForForLoop(move(flights));

    return 0;
}

