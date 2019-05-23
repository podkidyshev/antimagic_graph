#include <iostream>
#include <map>
#include <time.h>
#include <queue>
#include <exception>
#include <algorithm>

#include <chrono>

#include "perms.h"
#include "graph.h"
#include "utils.h"

using namespace std;


Edge::Edge(const int a, const int b) {
    if (a < b) {
        this->a = a;
        this->b = b;
    } else {
        this->a = b;
        this->b = a;
    }
}

bool Edge::operator<(const Edge &right) const {
    if (this->b != right.b)
        return this->b < right.b;
    else
        return this->a < right.a;
}

string Edge::to_string() {
    return "(" + std::to_string(this->a + 1) + " : " + std::to_string(this->b + 1) + ")";
}

Graph::Graph(int n, Edges edges) {
    this->n = n;
    this->m = edges.size();
    this->matrix = new int *[n];
    for (int i = 0; i < n; ++i) {
        this->matrix[i] = new int[n];
        for (int j = 0; j < n; ++j)
            this->matrix[i][j] = 0;
    }
    for (Edge edge: edges) {
        this->matrix[edge.a][edge.b] = 1;
        this->matrix[edge.b][edge.a] = 1;
    }
}

Graph::Graph(string &graph6) {
    // initialization
    this->n = int(graph6[0]) - 63;
    this->m = 0;
    this->matrix = new int *[n];
    for (int i = 0; i < n; ++i) {
        this->matrix[i] = new int[n];
        // put zeros on main
        this->matrix[i][i] = 0;
    }
    // parsing
    int row = 0;
    int col = 1;
    for (int i = 1; i < graph6.size(); ++i) {
        auto isymbol = short(graph6[i]) - 63;
        for (int ibit = 5; ibit >= 0; --ibit) {
            int bit = (isymbol >> ibit) & 1;
            this->matrix[row][col] = bit;
            this->matrix[col][row] = bit;
            this->m += bit;
            // update indices
            row++;
            if (row == col) {
                row = 0;
                col++;
            }
            if (col == n) {
                init_adj_list();
                return;
            }
        }
    }
    throw runtime_error("Error decoding graph6: " + graph6);
}

Graph::~Graph() {
    for (int i = 0; i < this->n; ++i)
        delete[] this->matrix[i];
    delete[] this->matrix;
}

Edges Graph::get_edges() {
    Edges edges = Edges();
    for (int i = 0; i < this->n; ++i)
        for (int j = 0; j < i; ++j)
            if (this->matrix[i][j])
                edges.emplace_back(i, j);
    return edges;
}

Edges Graph::get_divided_edges() {
    Edges edges;

    vector<int> js(this->n, 0);
    while (edges.size() < this->m) {
        for (int i = this->n - 1; i >= 0; --i)
            for (int j = js[i]; j < i; ++j) {
                if (this->matrix[i][j]) {
                    edges.emplace_back(i, j);
                    js[i] += 1;
                    break;
                }
                js[i] += 1;
            }
    }
    return edges;
}

VecVertices Graph::get_adj_list() {
    VecVertices adj = VecVertices((unsigned long) this->n, Vertices());
    for (int i = 0; i < this->n; ++i)
        for (int j = 0; j < this->n; ++j)
            if (this->matrix[i][j])
                adj[i].push_back(j);
    return adj;
}

Vertices Graph::get_isolated() {
    Vertices vertices = Vertices();
    for (int i = 0; i < this->n; ++i) {
        bool isolated = true;
        for (int j = 0; j < this->n; ++j)
            if (this->matrix[i][j]) {
                isolated = false;
                break;
            }
        if (isolated)
            vertices.push_back(i);
    }
    return vertices;
}

void Graph::init_adj_list() {
    if (adj.empty())
        adj = get_adj_list();
}

void Graph::display() {
    for (int i = 0; i < this->n; ++i) {
        for (int j = 0; j < this->n; ++j)
            printf("%i ", this->matrix[i][j]);
        printf("\n");
    }
}

string Graph::to_graph6() {
    string g6(1, char(this->n + 63));
    int acc(0), bits(0);
    for (int j = 0; j < this->n; ++j)
        for (int i = 0; i < j; ++i) {
            acc <<= 1;
            acc += this->matrix[i][j];
            bits++;
            if (bits == 6) {
                g6 += char(acc + 63);
                acc = 0;
                bits = 0;
            }
        }
    if (bits > 0) {
        acc <<= (6 - bits);
        g6 += char(acc + 63);
    }
    for (char c: g6)
        cout << int(c) << ' ';
    cout << endl;
    return g6;
}

uint Graph::is_antimagic(int increment) {
    // 1st optimization: isolated vertices count must be <= 1
    if (this->get_isolated().size() > 1)
        return false;
    // 2nd optimization: if there is K2 connected component in G
    init_adj_list();
    for (int i = 0; i < this->n; ++i)
        if (adj[i].size() == 1)
            if (adj[adj[i][0]].size() == 1 && adj[adj[i][0]][0] == i)
                return false;

    Edges edges = this->get_divided_edges();
    PermGen gen(int(edges.size()), true, 200000);
    int *perm = gen.next();
    map<Edge, int> phi = map<Edge, int>();
    vector<int> f = vector<int>(this->n, 0);

    while (perm != nullptr) {
        for (int i = 0; i < edges.size(); ++i)
            phi[edges[i]] = perm[i] + 1 + increment;  // phi: E -> [1, 2, ..., edges_count]

        // calc sum of phi for every vertex
        for (int i = 0; i < this->n; ++i) {
            f[i] = 0;
            for (int j = 0; j < this->n; ++j)
                if (this->matrix[i][j])
                    f[i] += phi[Edge(i, j)];
        }

        // looking for duplicates
        sort(f.begin(), f.end());
        bool antimagic = true;
        for (int i = 0; i < this->n - 2; ++i)
            if (f[i] == f[i + 1]) {
                antimagic = false;
                break;
            }

        // return true if antimagic phi was found
        if (antimagic)
            return true;
        perm = gen.next();
    }
    // return false if antimagic phi was never found
    return false;
}

uint Graph::get_distance(Vertex u, Vertex v) {
    vector<uint> distances(this->n, INFINITE_DISTANCE);
    distances[u] = 0;
    queue<Vertex> q;
    q.push(u);

    while (!q.empty()) {
        Vertex t = q.front();
        q.pop();
        for (Vertex ti = 0; ti < this->n; ++ti)
            if (this->matrix[t][ti] == 1 && distances[ti] == INFINITE_DISTANCE) {
                if (ti == v)
                    return distances[t] + 1;
                q.push(ti);
                distances[ti] = distances[t] + 1;
            }
    }
    throw runtime_error("Vertex is unreachable");
}

vector<uint> Graph::get_all_distances(Vertex u, set<Vertex> *ignored) {
    if (ignored == nullptr)
        ignored = new set<Vertex>();

    vector<uint> distances(this->n, INFINITE_DISTANCE);
    distances[u] = 0;
    queue<Vertex> q;
    q.push(u);

    while (!q.empty()) {
        Vertex t = q.front();
        q.pop();
        for (Vertex ti = 0; ti < this->n; ++ti)
            if (this->matrix[t][ti] == 1 && distances[ti] == INFINITE_DISTANCE) {
                if (ignored->find(ti) != ignored->end())
                    continue;
                q.push(ti);
                distances[ti] = distances[t] + 1;
            }
    }
    return distances;
}

Vertices Graph::get_path(Vertex u, Vertex v) {
    Vertices ancestors(this->n, this->n + 1);
    ancestors[u] = u;
    queue<Vertex> q;
    q.push(u);

    while (!q.empty()) {
        Vertex t = q.front();
        if (t == v)
            break;
        q.pop();
        for (Vertex ti = 0; ti < this->n; ++ti)
            if (this->matrix[t][ti] == 1 && ancestors[ti] == this->n + 1) {
                q.push(ti);
                ancestors[ti] = t;
            }
    }

    Vertices path;
    path.push_back(v);
    Vertex cur = v;
    while (ancestors[cur] != cur) {
        path.push_back(ancestors[cur]);
        cur = ancestors[cur];
    }
    reverse(path.begin(), path.end());
    return path;
}

bool Graph::is_connected() {
    vector<uint> distances = get_all_distances(0);
    for (uint d : distances)
        if (d == INFINITE_DISTANCE)
            return false;
    return true;
}
