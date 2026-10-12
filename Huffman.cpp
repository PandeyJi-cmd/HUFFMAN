// implement huffman algorithm.
#pragma once
#include <cstdint>
#include <vector>
#include <queue>
#include <algorithm>
#include <stack>
#include <array>


class Node{
    uint64_t bit;
    uint8_t c; // uint8_tacter represented by this node.
    bool leaf=false;
    public: 
    Node* l;
    Node* r;
    Node();
    ~Node(){
        delete l;
        delete r;
    }
    Node(uint8_t val,bool is_leaf,Node* left=nullptr,Node* right=nullptr){
        c=val;
        l=left;
        r=right;
        leaf=is_leaf;
        bit=-1;
    }
    bool is_leaf(){
         return leaf;
    }
    void set_bit(uint64_t b){
        bit=b; 
    }
    uint64_t get_bit(){return bit;}
    uint8_t getc(){return c;}
};
std::array<uint64_t,256> create_encoding(uint64_t freq_buff[],int size=256){
    std::priority_queue<std::pair<uint64_t,Node*>,std::vector<std::pair<uint64_t,Node*>>,std::greater<>>pq;
    for(size_t i=0;i<size;i++){
        if(freq_buff[i]!=0){
            Node *z = new Node(i,true);
            pq.push({freq_buff[i],z});
        }
    }
    // now take each element in pairs of two to create the new encoding. //

    while(pq.size()>1){
        uint64_t f1 = pq.top().first;
        Node* f=pq.top().second;
        pq.pop();
        uint64_t s1 = pq.top().first;
        Node* s = pq.top().second;
        pq.pop();
        s1+=f1;
        s =new Node('_',false,s,f);
        pq.push({s1,s});
    }

    // created the tree // now create the mapping.


    std::array<uint64_t,256> count = {0};
    if (pq.empty()) return count; // Guard against empty input

    Node* root = pq.top().second;
    pq.pop();

    std::stack<Node*> a;
    root->set_bit(1);
    a.push(root);
    while(!a.empty()){
        Node * k = a.top();
        uint64_t bit = k->get_bit();
        a.pop();
        uint8_t c = k->getc();
        if(!k->is_leaf()){
            if(k->r) {
                k->r->set_bit(bit<<1);
                a.push(k->r);
            }
            if(k->l) {
                k->l->set_bit(bit<<1 | 1);
                a.push(k->l);
            }
        }
        else{
            count[c] = (bit==1) ? 0b10 : bit;
        }
    }
    delete root; 
    return count;
}
