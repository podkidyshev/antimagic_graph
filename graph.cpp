#include <iostream>
#include <map>
#include <algorithm>

#include "perms.h"
#include "graph.h"


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

Graph::Graph(std::string &graph6) {
    // initialization
    this->n = int(graph6[0]) - 63;
    this->matrix = new int*[n];
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
            // update indices
            row++;
            if (row == col) {
                row = 0;
                col++;
            }
            if (col == n)
                return;
        }
    }
    raise(6);
}

Graph::~Graph() {
    for (int i = 0; i < this->n; ++i)
        delete [] this->matrix[i];
    delete [] this->matrix;
}

Edges Graph::get_edges() {
    Edges edges = Edges();
    for (int i = 0; i < this->n; ++i)
        for (int j = 0; j < i; ++j)
            if (this->matrix[i][j])
                edges.emplace_back(i, j);
    return edges;
}

void Graph::display() {
    for (int i = 0; i < this->n; ++i) {
        for (int j = 0; j < this->n; ++j)
            std::printf("%i ", this->matrix[i][j]);
        std::printf("\n");
    }
}

bool Graph::is_antimagic() {
    Edges edges = this->get_edges();
    PermGen gen = PermGen(int(edges.size()));
    int* perm = gen.next();

    while (perm != nullptr) {
        std::map<Edge, int> phi = std::map<Edge, int>();
        for (int i = 0; i < edges.size(); ++i)
            phi[edges[i]] = perm[i] + 1;  // ro2(phi) = [1, 2, ..., edges_count]

        // calc sum of phi for every vertex
        int* sums = new int[this->n];
        for (int i = 0; i < this->n; ++i) {
            int sum = 0;
            for (int j = 0; j < this->n; ++j)
                if (this->matrix[i][j])
                    sum += phi[Edge(i, j)];
            sums[i] = sum;
        }
        std::vector<int> vec = std::vector<int>();
        for (int i = 0; i < this->n; ++i)
            vec.push_back(sums[i]);
        delete [] sums;
        // looking for duplicates
        std::sort(vec.begin(), vec.end());
        bool antimagic = true;
        for (int i = 0; i < this->n - 2; ++i)
            if (sums[i] == sums[i + 1]) {
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
