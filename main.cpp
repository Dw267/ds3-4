#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <iomanip>

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

class GraphAdjacencyList {
 private:
  vector<AdjList> lists;
  unordered_map<string, int> idMap;
  int nodeCount = 0;

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
      AdjList temp;
      temp.putID = id;
      lists.push_back(temp);
      idMap[id] = lists.size() - 1;
    }
  }

  void AddEdge(string putID, string getID, float weight) {
    AddID(putID);
    AddID(getID);

    AdjNode temp;
    temp.getID = getID;
    temp.weight = weight;

    lists[idMap[putID]].nodes.push_back(temp);
    nodeCount++;
  }

  string MakeBinFileName(string fileNum) {
    return "pairs" + fileNum + ".bin";
  }

  string MakeAdjFileName(string fileNum) {
    return "pairs" + fileNum + ".adj";
  }

  void SortLists() {
    for (int i = 0; i < lists.size(); i++) {
      sort(lists[i].nodes.begin(), lists[i].nodes.end(),
           [](const AdjNode& a, const AdjNode& b) {
             return a.getID < b.getID;
           });
    }

    sort(lists.begin(), lists.end(),
         [](const AdjList& a, const AdjList& b) {
           return a.putID < b.putID;
         });

    idMap.clear();

    for (int i = 0; i < lists.size(); i++) {
      idMap[lists[i].putID] = i;
    }
  }

  void WriteAdjFile(string fileNum) {
    string filename = MakeAdjFileName(fileNum);
    ofstream fout(filename);

    for (int i = 0; i < lists.size(); i++) {
      fout << "[" << setw(3) << i + 1 << "] " << lists[i].putID;

      for (int j = 0; j < lists[i].nodes.size(); j++) {
        fout << " -> " << lists[i].nodes[j].getID
             << " (" << fixed << setprecision(2)
             << lists[i].nodes[j].weight << ")";
      }

      fout << endl;
    }

    fout.close();
  }

 public:
  GraphAdjacencyList() {
    nodeCount = 0;
  }

  void Clear() {
    lists.clear();
    idMap.clear();
    nodeCount = 0;
  }

  bool BuildAdjacencyLists(string fileNum) {
    Clear();

    string filename = MakeBinFileName(fileNum);
    ifstream fin(filename, ios::binary);

    if (!fin) {
      cout << endl;
      cout << "### " << filename << " does not exist! ###" << endl;
      return false;
    }

    PairRecord one;

    while (fin.read((char*)&one, sizeof(PairRecord))) {
      string putID = CharToString(one.putID);
      string getID = CharToString(one.getID);
      AddEdge(putID, getID, one.weight);
    }

    fin.close();

    SortLists();
    WriteAdjFile(fileNum);

    cout << endl;
    cout << "<<< There are " << lists.size() << " IDs in total. >>>" << endl;
    cout << endl;
    cout << "<<< There are " << nodeCount << " nodes in total. >>>" << endl;

    return true;
  }
};

class DataStructureSystem {
 private:
  GraphAdjacencyList graph;

  void ShowMenu() {
    cout << endl;
    cout << "* Data Structures and Algorithms *" << endl;
    cout << "**** Graph data manipulation *****" << endl;
    cout << "* 0. QUIT                       *" << endl;
    cout << "* 1. Build adjacency lists      *" << endl;
    cout << "* 2. Compute connection counts  *" << endl;
    cout << "**********************************" << endl;
    cout << "Input a choice(0, 1, 2): ";
  }

  void Mission1() {
    string fileNum;

    cout << endl;
    cout << "Input a file number ([0] Quit): ";
    cin >> fileNum;

    if (fileNum == "0") {
      return;
    }

    graph.BuildAdjacencyLists(fileNum);
  }

 public:
  void Run() {
    int command;

    while (true) {
      ShowMenu();
      cin >> command;

      if (command == 0) {
        break;
      } else if (command == 1) {
        Mission1();
      } else if (command == 2) {
        cout << endl;
        cout << "### Command 2 is not implemented yet. ###" << endl;
      } else {
        cout << endl;
        cout << "### Command does not exist! ###" << endl;
      }
    }
  }
};

int main() {
  DataStructureSystem system;
  system.Run();

  return 0;
}