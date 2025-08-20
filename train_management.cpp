#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <algorithm>
#include <map>
#include <functional>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

// Structure for train scheduling
struct Train {
    int trainID;
    int arrivalTime;
    int departureTime;
    int platform;
    int capacity;
    int availableSeats;
    int currentTrack;
    int waitingForTrack;
    int priority;
};

struct Station {
    int stationID;
    string stationName;
};

struct Route {
    int trainID;
    vector<int> stations;
    vector<int> arrivalTimes;
    vector<int> departureTimes;
};

struct CompareTrain {
    bool operator()(const Train &a, const Train &b) {
        return a.departureTime > b.departureTime;
    }
};

void buildWaitForGraph(const vector<Train> &trains, vector<vector<int>> &waitForGraph) {
    for (int i = 0; i < trains.size(); ++i) {
        if (trains[i].waitingForTrack != -1) {
            for (int j = 0; j < trains.size(); ++j) {
                if (trains[j].currentTrack == trains[i].waitingForTrack) {
                    waitForGraph[i].push_back(j);
                }
            }
        }
    }
}

void scheduleTrains(vector<Train> &trains, int platforms) {
    vector<int> platformAvailability(platforms, 0);
    for (auto &train : trains) {
        bool scheduled = false;
        for (int p = 0; p < platforms; ++p) {
            if (train.arrivalTime >= platformAvailability[p]) {
                train.platform = p;
                platformAvailability[p] = train.departureTime;
                scheduled = true;
                break;
            }
        }
        if (!scheduled) {
            cout << "Train " << train.trainID << " could not be scheduled on any platform.\n";
        }
    }
    cout << "Train scheduling completed.\n";
}

void shortestJobFirst(vector<int> &tasks) {
    sort(tasks.begin(), tasks.end());
    cout << "Maintenance order: ";
    for (int task : tasks) {
        cout << task << " ";
    }
    cout << endl;
}

void dijkstra(vector<vector<pair<int, int>>> &graph, int start) {
    int n = graph.size();
    vector<int> dist(n, INT_MAX);
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;

    dist[start] = 0;
    pq.push({0, start});

    while (!pq.empty()) {
        int d = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        for (auto &[v, weight] : graph[u]) {
            if (d + weight < dist[v]) {
                dist[v] = d + weight;
                pq.push({dist[v], v});
            }
        }
    }

    cout << "Shortest distances from station " << start << ": ";
    for (int d : dist) {
        cout << d << " ";
    }
    cout << endl;
}

bool detectCycle(int u, vector<vector<int>> &graph, vector<bool> &visited, vector<bool> &recStack) {
    visited[u] = true;
    recStack[u] = true;

    for (int v : graph[u]) {
        if (!visited[v] && detectCycle(v, graph, visited, recStack))
            return true;
        else if (recStack[v])
            return true;
    }
    recStack[u] = false;
    return false;
}

bool isDeadlocked(vector<vector<int>> &waitForGraph, int numTrains) {
    vector<bool> visited(numTrains, false);
    vector<bool> recStack(numTrains, false);

    for (int i = 0; i < numTrains; ++i) {
        if (!visited[i] && detectCycle(i, waitForGraph, visited, recStack))
            return true;
    }
    return false;
}

void resolveDeadlock(vector<vector<int>> &waitForGraph, vector<Train> &trains) {
    int n = trains.size();
    vector<bool> visited(n, false);
    vector<bool> recStack(n, false);

    for (int i = 0; i < n; ++i) {
        if (!visited[i] && detectCycle(i, waitForGraph, visited, recStack)) {
            cout << "Deadlock detected. Resolving...\n";
            for (int j = 0; j < n; ++j) {
                if (recStack[j]) {
                    trains[j].departureTime += 5;
                    cout << "Train " << trains[j].trainID << " delayed by 5 minutes.\n";
                    return;
                }
            }
        }
    }
    cout << "No deadlock detected to resolve.\n";
}

void generateTimetable(const vector<Station>& stations, const vector<Route>& routes) {
    cout << "\n*** Train Timetable ***\n";
    for (const auto& route : routes) {
        cout << "Train ID: " << route.trainID << endl;
        cout << setw(15) << "Station" << setw(15) << "Arrival" << setw(15) << "Departure" << endl;
        cout << string(45, '-') << endl;

        for (size_t i = 0; i < route.stations.size(); ++i) {
            auto stationIt = find_if(stations.begin(), stations.end(),
                                     [&](const Station& s) { return s.stationID == route.stations[i]; });
            string stationName = (stationIt != stations.end()) ? stationIt->stationName : "Unknown";

            cout << setw(15) << stationName
                 << setw(15) << route.arrivalTimes[i]
                 << setw(15) << route.departureTimes[i] << endl;
        }
        cout << endl;
    }
}

void calculateFare(const vector<Station>& stations, const vector<Route>& routes) {
    int trainID;
    cout << "Enter the Train ID to calculate fare: ";
    cin >> trainID;

    auto routeIt = find_if(routes.begin(), routes.end(), [&](const Route& r) { return r.trainID == trainID; });

    if (routeIt == routes.end()) {
        cout << "Train not found.\n";
        return;
    }

    int totalDistance = 0;
    for (size_t i = 1; i < routeIt->stations.size(); ++i) {
        totalDistance += abs(routeIt->stations[i] - routeIt->stations[i - 1]);
    }

    string trainClass;
    cout << "Enter train class (First/Second): ";
    cin >> trainClass;

    int farePerKm = (trainClass == "First") ? 10 : 5;
    int totalFare = totalDistance * farePerKm;

    cout << "Total fare for Train " << trainID << ": " << totalFare << " units\n";
}

void recommendTrain(const vector<Train>& trains, const string& preference) {
    Train bestTrain;
    if (preference == "less time") {
        int minTime = INT_MAX;
        for (const auto& train : trains) {
            if (train.arrivalTime < minTime) {
                minTime = train.arrivalTime;
                bestTrain = train;
            }
        }
        cout << "Recommended Train based on 'Less Time':\n";
        cout << "Train ID: " << bestTrain.trainID << ", Arrival Time: " << bestTrain.arrivalTime << ", Departure Time: " << bestTrain.departureTime << endl;
    } else if (preference == "less cost") {
        int minCost = INT_MAX;
        for (const auto& train : trains) {
            if (train.availableSeats < minCost) {
                minCost = train.availableSeats;
                bestTrain = train;
            }
        }
        cout << "Recommended Train based on 'Less Cost':\n";
        cout << "Train ID: " << bestTrain.trainID << ", Available Seats: " << bestTrain.availableSeats << endl;
    } else if (preference == "both") {
        double bestScore = INT_MAX;
        for (const auto& train : trains) {
            double score = (train.arrivalTime * 1.0 / train.availableSeats);
            if (score < bestScore) {
                bestScore = score;
                bestTrain = train;
            }
        }
        cout << "Recommended Train based on 'Both Time and Cost':\n";
        cout << "Train ID: " << bestTrain.trainID << ", Arrival Time: " << bestTrain.arrivalTime << ", Available Seats: " << bestTrain.availableSeats << endl;
    } else {
        cout << "Invalid preference input.\n";
    }
}

void displayMenu(vector<Train> &trains, int platforms, vector<int> &tasks, vector<vector<pair<int, int>>> &graph, const vector<Station>& stations, const vector<Route>& routes) {
    int choice;
    do {
        cout << "\n*** Train Management System Menu ***\n";
        cout << "1. Schedule Trains\n";
        cout << "2. Perform Maintenance (Shortest Job First)\n";
        cout << "3. Find Shortest Paths (Dijkstra's Algorithm)\n";
        cout << "4. Detect Deadlock\n";
        cout << "5. Resolve Deadlock\n";
        cout << "6. Generate Timetable\n";
        cout << "7. Calculate Fare\n";
        cout << "8. Recommend Train (Time/Cost Preference)\n";
        cout << "0. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                scheduleTrains(trains, platforms);
                break;
            case 2:
                shortestJobFirst(tasks);
                break;
            case 3: {
                int start;
                cout << "Enter starting station (0 to " << graph.size() - 1 << "): ";
                cin >> start;
                if (start >= 0 && start < graph.size()) {
                    dijkstra(graph, start);
                } else {
                    cout << "Invalid station number.\n";
                }
                break;
            }
            case 4: {
                vector<vector<int>> waitForGraph(trains.size());
                buildWaitForGraph(trains, waitForGraph);
                if (isDeadlocked(waitForGraph, trains.size())) {
                    cout << "Deadlock detected.\n";
                } else {
                    cout << "No deadlock detected.\n";
                }
                break;
            }
            case 5: {
                vector<vector<int>> waitForGraph(trains.size());
                buildWaitForGraph(trains, waitForGraph);
                resolveDeadlock(waitForGraph, trains);
                break;
            }
            case 6:
                generateTimetable(stations, routes);
                break;
            case 7:
                calculateFare(stations, routes);
                break;
            case 8: {
                string preference;
                cout << "What do you prefer? (less time, less cost, both): ";
                cin >> preference;
                recommendTrain(trains, preference);
                break;
            }
            case 0:
                cout << "Exiting Train Management System. Goodbye!\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 0);
}



vector<Train> loadTrainsFromTxt(const string &filename) {
    vector<Train> trains;
    ifstream file(filename);
    string line;

    if (!file.is_open()) {
        cerr << "Error: Could not open the file " << filename << endl;
        return trains;
    }

    while (getline(file, line)) {
        stringstream ss(line);
        Train train;

        ss >> train.trainID
           >> train.arrivalTime
           >> train.departureTime
           >> train.platform
           >> train.capacity
           >> train.availableSeats
           >> train.currentTrack
           >> train.waitingForTrack
           >> train.priority;

        trains.push_back(train);
    }

    file.close();
    return trains;
}


int main() {
    int platforms = 3;
    int totalTracks = 5;

    vector<Train> trains = loadTrainsFromTxt("train_data.txt");


    vector<int> tasks = {5, 2, 3, 7};

    vector<Station> stations = {
        {0, "Station A"}, {1, "Station B"}, {2, "Station C"}
    };

    vector<Route> routes = {
        {0, {0, 1, 2}, {10, 15, 20}, {20, 25, 30}},
        {1, {0, 1}, {15, 20}, {25, 30}},
        {2, {1, 2}, {20, 30}, {30, 40}}
    };

    vector<vector<pair<int, int>>> graph(3);
    graph[0].push_back({1, 5});
    graph[1].push_back({2, 10});

    displayMenu(trains, platforms, tasks, graph, stations, routes);

    return 0;
}
