#include <iostream>
#include <vector>
#include <set>
#include <queue>

using namespace std;
struct Point {
    int x;
    int property;
    Point(int _x, int _p) {
        x = _x;
        property = _p;
    }
};
struct Edge {
    int a;
    int b;
    int property;
    Edge(int _a, int _b, int _p) {
        a = _a;
        b = _b;
        property = _p;
    }
};

struct Graph {
    vector<Point> sp;
    set<int>   setP;
    vector<vector<Edge>> g;
    int edgeSize;
    Graph() {
        edgeSize = 0;
    }

    bool addPoint(int point, int property) {
        if (hasPoint(point)) {
            return false;
        }
        setP.insert(point);
        sp.push_back(Point(point, property));
        return true;
    }

    bool addEdge(const Edge& e) {
        if (!hasPoint(e.a) || !hasPoint(e.b)) {
            return false;
        }
        ++edgeSize;
        if (g.size() <= e.a) {
            g.resize(e.a + 1);
        }
        g[e.a].push_back(e);
        return true;
    }
    int getPoint(int index) const {
        return sp[index].x;
    }
    int getPointSize() const {
        return sp.size();
    }

    int getTotalEdgeSize() const {
        return edgeSize;
    }
    
    int getNbrEdgeSize(int point) const {
        return g[point].size();
    }

    Edge getNbrEdge(int point, int index) const {
        return g[point][index];
    }

    int getNbrEdgePoint(int point, int index) const {
        return g[point][index].b;
    }

    int getNbrEdgeProperty(int point, int index) const {
        return g[point][index].property;
    }

    int hasPoint(int point) const {
       return setP.find(point) != setP.end();
    }

    void bfs(int p) {
        queue<int> q;
        q.push(p);
        vector<int> visited(sp.size(), false);
        visited[p] = true;
        while(!q.empty()) {
            int x = q.front();
            cout << x << endl;
            q.pop();
            for (int i = 0; i < this->getNbrEdgeSize(x); i += 1) {
                int to = this->getNbrEdgePoint(x, i);
                if (!visited[to]) {
                    visited[to] = true;
                    q.push(to);
                }
            }
        }
    }
};

struct Frontier {
    vector<Point> f;
    void push_back(Point x) {
        f.push_back(x);
    }
};

struct LeftOperand {
    Graph g;
    Frontier f;
    LeftOperand(Graph _g, Frontier _f) {
        g = _g;
        f = _f;
    }
    vector<Point>& getFrontier() {
        return f.f;
    }
};

struct LeftOperands {
    vector<LeftOperand> vL;
    int size() {
        return vL.size();
    }

    void push_back(const LeftOperand& lOp) {
        vL.push_back(lOp);
    }

    LeftOperand& operator[](int i) {
        return vL[i];
    }

    // Edge e means the in LftOp[e.a] connect to LftOp[e.b]
    LeftOperands EdgeAtATime( const Edge& e/*pattern edge*/, const Graph &g) {
        LeftOperands nextAnswer;
        for (auto &lOp: vL) {
            if (e.a >= lOp.g.getPointSize()) {
                cerr << "Incorrect a edge pattern should connect to previous point";
                return nextAnswer;
            }
            int sourcePoint = lOp.g.getPoint(e.a);
            if (e.b >= lOp.g.getPointSize()) {
                // 扩列
                for (int i = 0; i < g.getNbrEdgeSize(sourcePoint); i += 1) {
                    int to = g.getNbrEdgePoint(sourcePoint, i);
                    if (!lOp.g.hasPoint(to))
                    {
                        LeftOperand result = lOp;
                        result.g.addPoint(to, 0);
                        result.g.addEdge(Edge(sourcePoint, to, 0));
                        nextAnswer.push_back(result);
                    }
                }    
            }
            else {
                // 不扩列, 过滤
                int dstPoint = lOp.g.sp[e.b].x;
                bool foundDst = false;
                for (int i = 0; i < g.getNbrEdgeSize(sourcePoint); i += 1) {
                    if (g.getNbrEdgePoint(sourcePoint, i) == dstPoint) {
                        foundDst = true;
                        break;
                    }
                }
                if (foundDst) {
                    nextAnswer.push_back(lOp);
                }
            }
        }
        return nextAnswer;
    }

    LeftOperands PointAtATime(const vector<Point>& vp, const Graph &g) {
        LeftOperands nextAnswer;
        for (auto &lOp: vL) {
            if (vp.size() == 0) {
                return *this;
            }
            
            int pInData = lOp.g.getPoint(vp[0].x);
            set<int> expanding;
            for (int i = 0; i < g.getNbrEdgeSize(pInData); i += 1) { 
                expanding.insert(g.getNbrEdgePoint(pInData, i));
            }
            for (int i = 1; i < vp.size(); i += 1) {
                pInData = lOp.g.getPoint(vp[i].x);
                set<int> joinSet;
                for (int i = 0; i < g.getNbrEdgeSize(pInData); i += 1) {
                    int to = g.getNbrEdgePoint(pInData, i);
                    if (expanding.find(to) != expanding.end()) {
                        joinSet.insert(to);
                    }
                }
                expanding.swap(joinSet);
            }

            for (auto iter: expanding) {
                LeftOperand result = lOp;
                result.g.addPoint(iter,0);
                for (auto& pt: vp) {
                    int pInData = result.g.getPoint(pt.x);
                    result.g.addEdge(Edge(pInData, iter, 0));
                }
                nextAnswer.push_back(result);
            }
        }
        return nextAnswer;
    }

    LeftOperands BfsExpand(const Graph &g) {
        LeftOperands nextAnswer;
        for (auto &lOp : vL) {
            LeftOperand result = lOp;
            Frontier f;
            set<int> exist;
            for (const auto& point: result.getFrontier()) {
                int pt = point.x;
                for (int i = 0; i < g.getNbrEdgeSize(pt); i += 1) {
                    int to = g.getNbrEdgePoint(pt, i);
                    if (!result.g.hasPoint(to)) {
                        result.g.addPoint(to, point.property + 1);
                        result.g.addEdge(g.getNbrEdge(pt, i));
                        if (exist.find(to) == exist.end()) {
                            f.push_back(Point(to, point.property + 1));
                            exist.insert(to);
                        }
                    } 
                    else {
                        result.g.addEdge(g.getNbrEdge(pt, i));
                    }
                }
            }
            result.f = f;
            nextAnswer.vL.push_back(result);
        }
        return nextAnswer;
    }
};
/*******************************


********************************/

int main() {
    Graph g;
    for (int i = 0; i < 10; i += 1) {
        g.addPoint(i + 1, 0);
    }
    vector<Edge> edgeList = {{2,3,0},{2,4,0},{3,4,0},{4,5,0},{4,6,0},{5,6,0},{7,8,0},{7,9,0},{7,10,0},{8,9,0},{8,10,0},{9,10,0}};
    for (Edge &e: edgeList) {
        g.addEdge(e);
        swap(e.a, e.b);
        g.addEdge(e);
    }
    g.bfs(2);
    Graph start;
    start.addPoint(2, 0);
    Frontier f;
    f.push_back(Point(2, 0));
    LeftOperand lo(start, f);
    LeftOperands los;
    los.vL.push_back(lo);
    LeftOperands res = los.BfsExpand(g);
    cout << "pt size " << res.vL[0].g.getPointSize() << endl;
    cout << "edge size " << res.vL[0].g.getTotalEdgeSize() << endl;
    res = res.BfsExpand(g);
    cout << "pt size " << res.vL[0].g.getPointSize() << endl;
    cout << "edge size " << res.vL[0].g.getTotalEdgeSize() << endl;
    res = res.BfsExpand(g);
    cout << "pt size " << res.vL[0].g.getPointSize() << endl;
    cout << "edge size " << res.vL[0].g.getTotalEdgeSize() << endl;

   //========================================================
    cout << "Edge at a time pattern match" << endl;
    res = los.EdgeAtATime(Edge(0, 1, 0), g);
    cout << "answer set size " << res.size() << endl;
    for (int j = 0; j < res.size(); j += 1) {
        for (int i = 0; i < res[j].g.getPointSize(); i += 1) {
            cout << res[j].g.getPoint(i) << " ";
        }
        cout << endl;
    }
    res = res.EdgeAtATime(Edge(1, 2, 0), g);
    cout << "answer set size " << res.size() << endl;
    for (int j = 0; j < res.size(); j += 1) {
        for (int i = 0; i < res[j].g.getPointSize(); i += 1) {
            cout << res[j].g.getPoint(i) << " ";
        }
        cout << endl;
    }
    res = res.EdgeAtATime(Edge(0, 2, 0), g);
    cout << "answer set size " << res.size() << endl;
    for (int j = 0; j < res.size(); j += 1) {
        for (int i = 0; i < res[j].g.getPointSize(); i += 1) {
            cout << res[j].g.getPoint(i) << " ";
        }
        cout << endl;
    }
    //=======================================================
    cout << "Point at a time pattern match" << endl;
    res = los.PointAtATime({Point(0, 0)}, g);
    cout << "answer set size " << res.size() << endl;
    for (int j = 0; j < res.size(); j += 1) {
        for (int i = 0; i < res[j].g.getPointSize(); i += 1) {
            cout << res[j].g.getPoint(i) << " ";
        }
        cout << endl;
    }
    res = res.PointAtATime({Point(0, 0), Point(1, 0)}, g);
    cout << "answer set size " << res.size() << endl;
    for (int j = 0; j < res.size(); j += 1) {
        for (int i = 0; i < res[j].g.getPointSize(); i += 1) {
            cout << res[j].g.getPoint(i) << " ";
        }
        cout << endl;
    }
    return 0;
}