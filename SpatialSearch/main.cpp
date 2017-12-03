#include <iomanip>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <cmath>
#include <math.h>
#include <limits>

using namespace std;

#define debug(message) cerr << message << endl
#define print(message) cout << message << endl


static const double PI = 3.14159265358979323846;

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

	double distance(const Flight & other) {
		static const double earthRadiusKm = 6371.0;

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

	return Flight(0, move(callSign), stod(latitude), stod(longitude));
}




int main()
{
	ifstream input_file("code_challenge/20170901_080005.csv");

	if (!input_file.is_open()) {
		return 1;
	}

	map< string, size_t > indices;
	vector< Flight > flights;

	// parse the file
	string line;
	getline(input_file, line); // ignore first line;
	for (size_t n = 0; getline(input_file, line);) {
		try {
			Flight newFlight = parse_line(line);
			newFlight.index = n;
			++n;

			flights.push_back(move(newFlight));
			const auto & flight = flights.back();

			const auto indexResult = indices.insert(make_pair(flight.callSign, n));
			// if (!indexResult.second) {
			// 	throw runtime_error("Duplicate entry " + flight.callSign);
			// }
		}
		catch (runtime_error e) {
			debug(e.what());
			continue;
		}
	}

	if (flights.size() < 2) {
		// No reason to calculate distances
		return 0;
	}

	cout << "Blabla" << endl;

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

		cout << setfill(' ');
		cout << setw(8) << left << flight1->callSign << " ";
		cout << setw(8) << left << closest->callSign << " ";
		cout << setw(8) << right << fixed << setprecision(2) << min_distance;
		cout << endl;
	}

	cout << flights.size() << endl;
    return 0;
}

