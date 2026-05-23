#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <queue>    // 任務二需要 queue
#include <set>      // 用於收集不重複且排序的收訊者

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

// 任務二使用的結果結構
struct ConnectionResult {
    string putID;
    int count;
    vector<string> reachableIDs;
};

class GraphAdjacencyList {
private:
    vector<AdjList> lists;
    unordered_map<string, int> idMap;
    int edgeCount = 0; // 改名為 edgeCount 較精確

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

    // 任務二：執行 BFS 並回傳結果
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
                // 關鍵修改：只有當鄰居學號不是「自己」時，才列入連通清單
                if (neighbor.getID != res.putID) { 
                    visitedIDs.insert(neighbor.getID);
                }
                
                // BFS 的搜尋邏輯保持不變（依然要透過自己往後找，才能找到完整路徑）
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
        
        // 寫入 .adj 檔案
        ofstream fout("pairs" + fileNum + ".adj");
        fout << "<<< There are " << lists.size() << " IDs in total. >>>" << endl; // First line

        for (int i = 0; i < (int)lists.size(); i++) {
            fout << "[" << setw(3) << i + 1 << "] " << lists[i].putID << ": " << endl;
            if (!lists[i].nodes.empty()) {
                fout << "\t"; // 開頭縮排
                for (int j = 0; j < (int)lists[i].nodes.size(); j++) {
                    fout << "(" << setw(2) << j + 1 << ") " 
                         << lists[i].nodes[j].getID << "," 
                         << setw(7) << lists[i].nodes[j].weight;
                    
                    if (j < (int)lists[i].nodes.size() - 1) {
                        fout << "\t"; // 鄰居之間用 tab 分隔
                    }
                }
                fout << endl;
            }
        }
        fout << "<<< There are " << edgeCount << " nodes in total. >>>" << endl;
        fout.close();

        // 螢幕輸出
        cout << "\n<<< There are " << lists.size() << " IDs in total. >>>\n";
        cout << "<<< There are " << edgeCount << " nodes in total. >>>\n";
        return true;
    }

    void ComputeConnectionCounts(string fileNum) {
        if (IsEmpty()) {
            cout << "\n### Please build adjacency lists first (Mission 1). ###\n";
            return;
        }

        vector<ConnectionResult> results;
        // 對每個 ID 跑一次 BFS
        for (int i = 0; i < lists.size(); i++) {
            results.push_back(RunBFS(i));
        }

        // 任務二排序要求：連通數由大到小，學號由小到大
        sort(results.begin(), results.end(), [](const ConnectionResult& a, const ConnectionResult& b) {
            if (a.count != b.count) return a.count > b.count;
            return a.putID < b.putID;
        });

        // 寫入 .cnt 檔案
        ofstream fout("pairs" + fileNum + ".cnt");
        fout << "<<< There are " << results.size() << " IDs in total. >>>" << endl;

        for (int i = 0; i < (int)results.size(); i++) {
            // [編號] 學號(連通數): 換行
            fout << "[" << setw(3) << i + 1 << "] " << results[i].putID 
                 << "(" << results[i].count << "): " << endl;

            if (!results[i].reachableIDs.empty()) {
                for (int j = 0; j < (int)results[i].reachableIDs.size(); j++) {
                    // 每行開頭縮排 (如果是第一個或每 12 個一組的開頭)
                    if (j % 12 == 0) fout << "\t";

                    fout << "(" << setw(2) << j + 1 << ") " << results[i].reachableIDs[j];

                    // 每 12 個換行一次，或是在該學號清單結束時換行
                    if ((j + 1) == 12 && j == (int)results[i].reachableIDs.size() - 1) {
                        fout << "\n" << endl;
                    }
                    else if ((j + 1) % 12 == 0 || j == (int)results[i].reachableIDs.size() - 1) {
                        fout << endl;
                    } else {
                        fout << "\t"; // 資料間用 tab 分隔
                    }
                }
            }
        }
        fout.close();
        // Screen display
        cout << "\n<<< There are " << lists.size() << " IDs in total. >>>\n";
    }
};

class DataStructureSystem {
private:
    GraphAdjacencyList graph;
    string currentFileNum = "";

    void ShowMenu() {
        cout << "\n* Data Structures and Algorithms *" << endl;
        cout << "**** Graph data manipulation *****" << endl;
        cout << "* 0. QUIT                       *" << endl;
        cout << "* 1. Build adjacency lists      *" << endl;
        cout << "* 2. Compute connection counts  *" << endl;
        cout << "**********************************" << endl;
        cout << "Input a choice(0, 1, 2): ";
    }

public:
    void Run() {
        int command;
        while (true) {
            ShowMenu();
            if (!(cin >> command)) break;

            if (command == 0) break;
            else if (command == 1) {
                cout << "\nInput a file number ([0] Quit): ";
                cin >> currentFileNum;
                if (currentFileNum != "0") graph.BuildAdjacencyLists(currentFileNum);
            } else if (command == 2) {
                if (currentFileNum == "" || currentFileNum == "0") {
                    cout << "### There is no graph and choose 1 first. ###\n";
                } else {
                    graph.ComputeConnectionCounts(currentFileNum);
                }
            } else {
                cout << "\n### Command does not exist! ###\n";
            }
        }
    }
};

int main() {
    DataStructureSystem system;
    system.Run();
    return 0;
}