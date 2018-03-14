#pragma once

#include <bitset>
#include <iostream>
#include <list>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>

#ifdef __GNUC__
#define ffsll(x) __builtin_ffsll(x)
#else
#error No ffsll implementation available. Use gnu compiler or implement it
#endif

enum assign_mode 
{ 
    // only use stack
    ASG_NAIVE,

    // use registers, but never reuse registers
    ASG_NAIVE2,

    // reuse registers as much as possible using saturation alg
    ASG_SMART,

    // SMART but with bias towards using registers for more frequently
    // used vars
    ASG_RANGE,

    // Using Graph coloring algorithm (NP Comp, super slow)
    // with #reg colors, and increment by 1 every time it fails
    ASG_GCOLOR
};

enum stack_e
{
    STACK,
    ROOTSTACK
};


namespace Graph
{

// data structure to hold constraints of an unsigned int
struct Saturation
{
    std::vector<std::bitset<64> > sat;
    
    void insert(unsigned int val)
    {
        while (sat.size() <= val/64)
        {
            sat.push_back(std::bitset<64>());
            sat.back().set();
        }
        sat.at(val/64)[val%64] = 0;
    }

    unsigned int best()
    {
        unsigned int i;
        for (i = 0; i < sat.size(); i++)
        {
            int ffs = ffsll(sat.at(i).to_ullong());
            if (ffs != 0)
            {
                return i*64 + ffs-1;
            }
            else
            {
                continue;
            }
        }
        return i*64;
    }
};

struct Node
{
    Node(std::string s, stack_e stack_id) : name(s), n(-1), id(stack_id) { }
    void constrain(unsigned int constraint, stack_e constrainee_id, unsigned int reg_count)
    {
#ifdef DEBUG
        std::cout << name << " constraint: " << constraint << std::endl;
#endif
        if (constraint < reg_count || constrainee_id == id)
        {
            this->saturation.insert(constraint);
        }
    }
    unsigned int assign()
    {
        unsigned int best = saturation.best();
        this->n = best;
        return best;
    }
    // NOTE only use for naive methods
    void force(unsigned int v, unsigned int offset)
    {
        static int rootstack_count = 0;
        // FIXME this implies force at v = 0 should reset rootstack_count
        // it works for how I'm using it but if force were to be used to say
        // force register usage (similar to the register keyword in c), it'll break
        // things
        if (v == 0)
        {
            rootstack_count = 0;
        }
        if (id == STACK)
        {
            this->n = v + offset;
        }
        else
        {
            this->n = offset + rootstack_count++;
        }
    }
    std::pair<std::string, std::pair<unsigned int, stack_e> > get_mapping()
    {
        if (n == -1)
        {
            std::cerr << "Error trying to get mapping on an unassigned node\n";
        }
        return std::make_pair(this->name, std::make_pair(this->n, this->id));
    }
    // note: only directed, it is caller's responsibility to make sure both nodes get called
    void add_neighbor(Node* n)
    {
        neighbors.push_back(n);
    }

    // higher = the more we want it to be a register
    float preference() const
    {
        /* Criterias to think about:
         * 1) highest preference to loop variables
         * 2) higher preference to ones with constraints
         * 3) higher preference to low neighbors.size()
         */
        if (neighbors.size() != 0)
        {
            return 1.0/neighbors.size();
        }
        else
        {
            return 99999999.0f;
        }
    }

    std::string name;
    int n; // lower = register
    stack_e id;
    // awk way to ensure rootstack starts at 0 on force
    Saturation saturation;
    std::list<Node*> neighbors;
};

struct node_strict_less
{
    bool operator()(const Node* l, const Node* r)
    {
        return l->preference() < r->preference();
    }
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

    // stack_id abstractifies the rootstack vs normal stack
    // registers are shared, but stack is not
    void add_node(std::string a, stack_e stack_id)
    {
        m[a] = new Node(a, stack_id);
    }
    void add_edges(std::string src, std::list<std::string> dsts)
    {
        for (auto d : dsts)
        {
            m.at(src)->add_neighbor(m.at(d));
            m.at(d)->add_neighbor(m.at(src));
        }
    }
    std::unordered_map<std::string, std::pair<unsigned int, stack_e> > get_mapping()
    {
        std::unordered_map<std::string, std::pair<unsigned int, stack_e> > varmap;
        for (auto p : m)
        {
            varmap.insert(p.second->get_mapping());
        }
        return varmap;
    }
    void do_naive(int num_reg)
    {
        int i = 0;
        for (auto p : m)
        {
            p.second->force(i++, num_reg);
        }
    }
    void do_naive2(int num_reg)
    {
        int i = 0;
        for (auto p : m)
        {
            p.second->force(i++, 0);
        }
    }
    void do_smart(unsigned int num_reg)
    {
        std::priority_queue<Node*, std::vector<Node*>, node_strict_less > pq;
        for (auto p : m)
        {
            pq.push(p.second);
        }
        while(!pq.empty())
        {
            // a reasonably efficient way to manage the priority queue
            // when things keep changing due to adding constraints
            // only check if the top one is good enough compared to the second best one
            // this will not account for cases where both top and second top are not
            Node* top = pq.top();
            pq.pop();
            if (pq.top()->preference() <= top->preference())
            {
                unsigned int v = top->assign();
                for (Node* n : top->neighbors)
                {
                    n->constrain(v, top->id, num_reg);
                }
            }
            else
            {
                pq.push(top);
            }
        }
    }
    void assign(int num_reg, assign_mode mode = ASG_NAIVE)
    {
        switch (mode)
        {
            case ASG_NAIVE:
                do_naive(num_reg);
                break;
            case ASG_NAIVE2:
                do_naive2(num_reg);
                break;
            case ASG_SMART:
                do_smart(num_reg);
                break;
            case ASG_RANGE:
                std::cerr << "Error not supported assign_mode: " << mode << std::endl;
                break;
            case ASG_GCOLOR:
                std::cerr << "Error not supported assign_mode: " << mode << std::endl;
                break;
            default:
                std::cerr << "Error not valid assign_mode: " << mode << std::endl;
                break;
        }
    }

    std::unordered_map<std::string, Node*> m;

};

}
