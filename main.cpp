// 11220125 吳冠宏 11227237 梁智詮
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <queue>    // Queue for mission 2 BFS
#include <set>      // Set for collecting unique and sorted recipients

using namespace std;

struct PairRecord {
    char putID[12];
    char getID[12];
    float weight;
};

struct AdjNode {
    string getID;
    float weight;
};

struct AdjList {
    string putID;
    vector<AdjNode> nodes;
};

// This strcuture is build for mission 2, to store the BFS result for each ID
struct ConnectionResult {
    string putID;
    int count;
    vector<string> reachableIDs;
};

class GraphAdjacencyList {
private:
    vector<AdjList> lists;
    unordered_map<string, int> idMap;
    int edgeCount = 0; // Edge counting

    string CharToString(char id[12]) {
        string s = "";
        for (int i = 0; i < 12; i++) {
            if (id[i] == '\0') break;
            s += id[i];
        }
        return s;
    }

    void AddID(string id) {
        if (idMap.find(id) == idMap.end()) {
            idMap[id] = lists.size();
            AdjList temp;
            temp.putID = id;
            lists.push_back(temp);
        }
    }

    void AddEdge(string putID, string getID, float weight) {
        AddID(putID);
        AddID(getID);
        AdjNode temp;
        temp.getID = getID;
        temp.weight = weight;
        lists[idMap[putID]].nodes.push_back(temp);
        edgeCount++;
    }

    void SortLists() {
        for (auto &list : lists) {
            sort(list.nodes.begin(), list.nodes.end(), [](const AdjNode& a, const AdjNode& b) {
                return a.getID < b.getID;
            });
        }
        sort(lists.begin(), lists.end(), [](const AdjList& a, const AdjList& b) {
            return a.putID < b.putID;
        });
        idMap.clear();
        for (int i = 0; i < lists.size(); i++) {
            idMap[lists[i].putID] = i;
        }
    }

    // Mission 2: BFS to find all reachable IDs from given start index
    ConnectionResult RunBFS(int startIdx) {
        ConnectionResult res;
        res.putID = lists[startIdx].putID;
        set<string> visitedIDs;
        queue<int> q;
        vector<bool> visited(lists.size(), false);
        
        visited[startIdx] = true;
        q.push(startIdx);
        
        while (!q.empty()) {
            int currIdx = q.front();
            q.pop();
            for (const auto& neighbor : lists[currIdx].nodes) {
                // Only add to visitedIDs if it's not the starting ID itself, to avoid counting self-loop as a connection
                if (neighbor.getID != res.putID) { 
                    visitedIDs.insert(neighbor.getID);
                }
                
                // The basic BFS part to explore neighbors
                int neighborIdx = idMap[neighbor.getID];
                if (!visited[neighborIdx]) {
                    visited[neighborIdx] = true;
                    q.push(neighborIdx);
                }
            }
        }
        res.count = (int)visitedIDs.size();
        for (const string& id : visitedIDs) res.reachableIDs.push_back(id);
        return res;
    }

public:
    void Clear() {
        lists.clear();
        idMap.clear();
        edgeCount = 0;
    }

    bool IsEmpty() { return lists.empty(); }

    bool BuildAdjacencyLists(string fileNum) {
        Clear();
        string filename = "pairs" + fileNum + ".bin";
        ifstream fin(filename, ios::binary);
        if (!fin) {
            cout << "\n### " << filename << " does not exist! ###\n";
            return false;
        }

        PairRecord one;
        while (fin.read((char*)&one, sizeof(PairRecord))) {
            AddEdge(CharToString(one.putID), CharToString(one.getID), one.weight);
        }
        fin.close();

        SortLists();
        
        // Output .adj file
        ofstream fout("pairs" + fileNum + ".adj");
        fout << "<<< There are " << lists.size() << " IDs in total. >>>" << endl; // First line

        for (int i = 0; i < (int)lists.size(); i++) {
            // School ID line
            fout << "[" << setw(3) << i + 1 << "] " << lists[i].putID << ": " << endl;
            if (!lists[i].nodes.empty()) {
                for (int j = 0; j < (int)lists[i].nodes.size(); j++) {
                    // Indent at the beginning of the line (if it's the first neighbor or every 12 neighbors)
                    if (j % 12 == 0) fout << "\t";

                    fout << "(" << setw(2) << j + 1 << ") " 
                         << lists[i].nodes[j].getID << "," 
                         << setw(7) << lists[i].nodes[j].weight;
                    
                    // Change line every 12 neightbors or at the end of the neighbor list
                    if ((j + 1) % 12 == 0 && j == (int)lists[i].nodes.size() - 1) {
                        fout << "\n"<< endl;
                    } else if ((j + 1) % 12 == 0 || j == (int)lists[i].nodes.size() - 1){
                        fout << endl;
                    } else {
                        fout << "\t"; // Same line separation
                    }
                }
            }
        }
        fout << "<<< There are " << edgeCount << " nodes in total. >>>" << endl;
        fout.close();

        // Screen display
        cout << "\n<<< There are " << lists.size() << " IDs in total. >>>\n\n";
        cout << "<<< There are " << edgeCount << " nodes in total. >>>\n";
        return true;
    }

    void ComputeConnectionCounts(string fileNum) {
        if (IsEmpty()) {
            cout << "### There is no graph and choose 1 first. ###\n";
            return;
        }

        vector<ConnectionResult> results;
        // Run a BFS on each ID
        for (int i = 0; i < lists.size(); i++) {
            results.push_back(RunBFS(i));
        }

        // Mission 2 sorting requirement: connection count from large to small, ID from small to large
        sort(results.begin(), results.end(), [](const ConnectionResult& a, const ConnectionResult& b) {
            if (a.count != b.count) return a.count > b.count;
            return a.putID < b.putID;
        });

        // Output .cnt file
        ofstream fout("pairs" + fileNum + ".cnt");
        fout << "<<< There are " << results.size() << " IDs in total. >>>" << endl;

        for (int i = 0; i < (int)results.size(); i++) {
            // School ID line with connection count
            fout << "[" << setw(3) << i + 1 << "] " << results[i].putID 
                 << "(" << results[i].count << "): " << endl;

            if (!results[i].reachableIDs.empty()) {
                for (int j = 0; j < (int)results[i].reachableIDs.size(); j++) {
                    // Indent at the beginning of the line
                    if (j % 12 == 0) fout << "\t";

                    fout << "(" << setw(2) << j + 1 << ") " << results[i].reachableIDs[j];

                    // Change line every 12 neightbors
                    if ((j + 1) % 12 == 0 && j == (int)results[i].reachableIDs.size() - 1) {
                        fout << "\n" << endl;
                    } else if ((j + 1) % 12 == 0 || j == (int)results[i].reachableIDs.size() - 1) {
                        fout << endl;
                    } else {
                        fout << "\t"; // Seperate datas with tab
                    }
                }
            }
        }
        fout.close();
        // Screen display
        cout << "\n<<< There are " << lists.size() << " IDs in total. >>>\n";
    }
};

// Main System body
class DataStructureSystem {
private:
    GraphAdjacencyList graph;
    string currentFileNum = "";
    // Menu display
    void ShowMenu() {
        cout << "\n* Data Structures and Algorithms *" << endl;
        cout << "**** Graph data manipulation *****" << endl;
        cout << "* 0. QUIT                        *" << endl;
        cout << "* 1. Build adjacency lists       *" << endl;
        cout << "* 2. Compute connection counts   *" << endl;
        cout << "**********************************" << endl;
        cout << "Input a choice(0, 1, 2): ";
    }

public:
    void Run() {
        string command;
        while (true) {
            ShowMenu();
            cin >> command;
            if (command == "0") break;
            else if (command == "1") {
                cout << "\nInput a file number ([0] Quit): ";
                cin >> currentFileNum;
                if (currentFileNum != "0") graph.BuildAdjacencyLists(currentFileNum);
            } else if (command == "2") {
                if (currentFileNum == "" || currentFileNum == "0") {
                    cout << "### There is no graph and choose 1 first. ###\n";
                } else {
                    graph.ComputeConnectionCounts(currentFileNum);
                }
            } else {
                cout << "\nCommand does not exist!\n";
            }
        }
    }
};

// Main function
int main() {
    DataStructureSystem system;
    system.Run();
    return 0;
}