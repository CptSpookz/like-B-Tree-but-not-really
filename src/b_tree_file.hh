/* The following program performs deletion on a B-Tree. It contains functions
   specific for deletion along with all the other functions provided in the
   previous articles on B-Trees. See http://www.geeksforgeeks.org/b-tree-set-1-introduction-2/
   for previous article.
 
   The deletion function has been compartmentalized into 8 functions for ease
   of understanding and clarity
 
   The following functions are exclusive for deletion
   In class BTreeNode:
    1) remove
    2) removeFromLeaf
    3) removeFromNonLeaf
    4) getPred
    5) getSucc
    6) borrowFromPrev
    7) borrowFromNext
    8) merge
    9) findKey
 
   In class BTree:
     1) remove
 
  The removal of a key from a B-Tree is a fairly complicated process. The program handles
  all the 6 different cases that might arise while removing a key.
 
  Testing: The code has been tested using the B-Tree provided in the CLRS book( included
  in the main function ) along with other cases.
 
  Reference: CLRS3 - Chapter 18 - (499-502)
  It is advised to read the material in CLRS before taking a look at the code. */

#define DEBUG true

#if DEBUG == true
#include <iostream>
#endif

#include <fstream>
#include <cstring>
#include <string>

// A BTree file node
class BTreeNode
{
    int *keys;          // An array of keys
    int t;              // Maximum degree (defines the range for number of keys)
    int *C;             // And array of child file positions
    int n;              // Current number of keys
    bool leaf;          // Is true when node is leaf. Otherwise false

public:
    BTreeNode(int _t, bool _leaf);  // Constructor

    // A function to traverse all nodes in a subtree rooted with this node
    void traverse();

    // A function to search a key in subtree rooted with this node
    BTreeNode *search(int k);       // returns NULL if k is not present

    // A function to serialize node data
    char* serialize();

    // A function to deserialize node data
    void deserialize(char* data);

    // A function to get the predecessor of the key- where the key
    // is present in the idx-th position in the node
    int getPred(int idx);

    // A function to get the successor of the key- where the key
    // is present in the idx-th position in the node
    int getSucc(int idx);
};

// A file BTree
class BTree
{
    int root;               // Root file position
    int t;                  // Maximum degree
    BTreeNode* node;        // Current loaded node
    std::string fpath;      // File path
    std::fstream file;      // File stream (input/output, binary)

public:

    BTree(std::string _fpath);      // Constructor

    // A function to load BTree info from the file header
    void load_info_header();

    // A function to store BTree info in the file header
    void store_info_header(int _root, int _t);

    // A function to initialize Btree and file
    void init(int _t);

    // A function to get the next node, using the predecessor pointer on index
    void getPred(int idx);

    // A function to get the next node, using the successor pointer on index
    void getSucc(int idx);

    // A function to search key on tree
    void search(int key);
};

BTreeNode::BTreeNode(int _t, bool _leaf){
    this->t = _t;
    this->leaf = _leaf;
    this->n = 0;
    this->keys = new int [62];
    this->C = new int [63];
}

// BTreeNode definitions
void BTreeNode::traverse(){}

BTreeNode* BTreeNode::search(int k){}

char* BTreeNode::serialize(){
    char* data = new char[512];
    int data_idx = 0;

    memset(data, 0, 512);

    // Serializes (t, n, keys, children, leaf)
    memcpy(&data[data_idx], &this->t, sizeof(int));
    data_idx += sizeof(int);
    memcpy(&data[data_idx], &this->n, sizeof(int));
    data_idx += sizeof(int);
    memcpy(&data[data_idx], this->keys, sizeof(int)*62);
    data_idx += sizeof(int)*62;
    memcpy(&data[data_idx], this->C, sizeof(int)*63);
    data_idx += sizeof(int)*63;
    memcpy(&data[data_idx], &this->leaf, sizeof(bool));

    return data;
}

void BTreeNode::deserialize(char* data){
    int data_idx = 0;

    // Deserializes (t, n, keys, children, leaf)
    memcpy(&this->t, &data[data_idx], sizeof(int));
    data_idx += sizeof(int);
    memcpy(&this->n, &data[data_idx], sizeof(int));
    data_idx += sizeof(int);
    memcpy(this->keys, &data[data_idx], sizeof(int)*62);
    data_idx += sizeof(int)*62;
    memcpy(this->C, &data[data_idx], sizeof(int)*63);
    data_idx += sizeof(int)*63;
    memcpy(&this->leaf, &data[data_idx], sizeof(bool));
}

int BTreeNode::getPred(int idx){}

int BTreeNode::getSucc(int idx){}


// BTree definitions
BTree::BTree(std::string _fpath){
    this->fpath = _fpath;
    this->file = std::fstream(_fpath, std::fstream::in | std::fstream::out | std::fstream::binary);
}

void BTree::load_info_header(){
    if(this->file.is_open()){
        char* buffer = new char[sizeof(int)*2];

        if(DEBUG == true){
            std::cout << "Reading info header data" << std::endl;
        }

        this->file.seekg(0, this->file.beg);
        this->file.read(buffer, sizeof(int)*2);

        memcpy(&this->root, buffer, sizeof(int));
        memcpy(&this->t, &buffer[sizeof(int)], sizeof(int));

        if(DEBUG == true){
            std::cout << "Root position: " << this->root << std::endl;
            std::cout << "Minimum degree: " << this->t << std::endl;
        }
    
        delete buffer;
    }
}

void BTree::store_info_header(int _root, int _t){
    if(this->file.is_open()){
        char* buffer = new char[sizeof(int)];
        memcpy(buffer, &_root, sizeof(int));
        memcpy( &buffer[sizeof(int)], &_t, sizeof(int));
 
        if(DEBUG == true)
            std::cout << "Writing info header data" << std::endl;

        this->file.seekp(0, this->file.beg);
        this->file.write(buffer, sizeof(int)*2);

        delete buffer;
    }
}

void BTree::init(int _t){
    if(this->file.is_open()){
        // initializes with root on 0
        this->store_info_header(0, _t);
       
        this->node = new BTreeNode(_t, true); 
        char *root = this->node->serialize();

        if(DEBUG == true)
            std::cout << "Initializing BTree root" << std::endl;

        this->file.write(root, 512);

        delete root;
    }
}

void BTree::getPred(int idx){
    if(this->file.is_open()){
        int ptr = this->node->getPred(idx);
        if(DEBUG == true)
            std::cout << "Getting node on relative location: " << ptr << std::endl;
    }
}

void BTree::getSucc(int idx){
    if(this->file.is_open()){
        int ptr = this->node->getSucc(idx);
        if(DEBUG == true)
            std::cout << "Getting node on relative location: " << ptr << std::endl;
    }
}

void BTree::search(int key){

}
