#pragma once

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>

enum assign_mode 
{ 
    ASG_NAIVE, 
    ASG_NAIVE2, 
    ASG_SMART, 
    ASG_RANGE 
};


namespace Graph
{

struct Node
{
    Node(std::string s) : name(s), n(-1) { }
    void constrain(unsigned int constraint)
    {
        this->saturation.insert(constraint);
    }
    unsigned int assign()
    {
        auto it = this->saturation.begin();
        this->n = *it;
        return *it;
    }
    // NOTE only use for naive methods
    void force(unsigned int v)
    {
        this->n = v;
    }
    std::pair<std::string, unsigned int> get_mapping()
    {
        if (n == -1)
        {
            std::cerr << "Error trying to get mapping on an unassigned node\n";
        }
        return std::make_pair(this->name, this->n);
    }
    // note: only directed, it is caller's responsibility to make sure both nodes get called
    void add_neighbor(Node* n)
    {
        neighbors.push_back(n);
    }

    std::string name;
    int n; // lower = register
    std::set<unsigned int> saturation;
    std::list<Node*> neighbors;
};

struct NodeList
{
    NodeList() { }
    ~NodeList()
    {
        for (auto p : m)
        {
            delete p.second;
        }
    }

    void add_node(std::string a)
    {
        m[a] = new Node(a);
    }
    void add_edges(std::string src, std::list<std::string> dsts)
    {
        for (auto d : dsts)
        {
            m.at(src)->add_neighbor(m.at(d));
            m.at(d)->add_neighbor(m.at(src));
        }
    }
    std::map<std::string, unsigned int> get_mapping()
    {
        std::map<std::string, unsigned int> varmap;
        for (auto p : m)
        {
            varmap.insert(p.second->get_mapping());
        }
        return varmap;
    }
    void assign(int num_reg, assign_mode mode = ASG_NAIVE)
    {
        switch (mode)
        {
            case ASG_NAIVE:
                {
                    int i = 0;
                    for (auto p : m)
                    {
                        p.second->force(i++ + num_reg);
                    }
                }
                break;
            case ASG_NAIVE2:
                {
                    int i = 0;
                    for (auto p : m)
                    {
                        p.second->force(i++);
                    }
                }
                break;
            case ASG_SMART:
                std::cerr << "Error not supported assign_mode: " << mode << std::endl;
                break;
            case ASG_RANGE:
                std::cerr << "Error not supported assign_mode: " << mode << std::endl;
                break;
            default:
                std::cerr << "Error not valid assign_mode: " << mode << std::endl;
                break;
        }
    }

    std::map<std::string, Node*> m;

};

}
