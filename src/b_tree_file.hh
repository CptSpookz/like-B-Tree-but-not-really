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

    // A function to check if BTree is empty
    bool is_empty();

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

    // Access to private attributes
    friend class BTree;
};

// A file BTree
class BTree
{
    int root;               // Root file position
    int t;                  // Maximum degree
    BTreeNode* node;        // Current loaded node
    int node_ptr;           // Current node pointer
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

    // A function to load a node from secondary memory to the primary memory,
    // using a pointer
    void load_node(int ptr);

    // A function to store a node from primary memory to secondary memory,
    // using a pointer
    void store_node(int ptr, BTreeNode node);

    // A function to add a new node to secondary memory
    // Returns file pointer
    int add_node(BTreeNode node);

    // A function to insert key k
    void insert(int key);

    // A function to insert key k in a non-full node
    void insertNonFull(int key);

    // A function to split full nodes
    void splitChild(int i, BTreeNode* y, BTreeNode* p);

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
bool BTreeNode::is_empty(){
    return (this->n == 0);
}

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

void BTree::load_node(int ptr){
    if(this->file.is_open()){
        // Checks size of file
        this->file.seekg(0, this->file.end);
        int size = this->file.tellg();

        // Check if pointer is valid on file
        if((size-sizeof(int)*2)/512 >= ptr){
            this->node_ptr = ptr;
            char data[512];

            if(DEBUG == true)
                std::cout << "Loading node data from " << sizeof(int)*2+ptr*512 << std::endl;

            this->file.seekg(sizeof(int)*2+ptr*512, this->file.beg);
            this->file.read(data, 512);            

            this->node->deserialize(data);
        }
    }
}

void BTree::store_node(int ptr, BTreeNode node){
    if(this->file.is_open()){
        // Checks size of file
        this->file.seekg(0, this->file.end);
        int size = this->file.tellg();
        // Check if pointer is valid on file
        if(((size-sizeof(int)*2)/512)-1 >= ptr){
            char* data = node.serialize();

            if(DEBUG == true)
                std::cout << "Storing node data on " << sizeof(int)*2+ptr*512 << std::endl;

            this->file.seekg(sizeof(int)*2+ptr*512, this->file.beg);
            this->file.write(data, 512);
        }
    }
}

int BTree::add_node(BTreeNode node){
    if(this->file.is_open()){
        char *data = node.serialize();

        this->file.seekp(0, this->file.end);
    
        int ptr = (this->file.tellp()-(sizeof(int)*2))/512;
    
        this->file.write(data, 512);
        
        return ptr;
    }
    return -1;
}

void BTree::insert(int key){
    if(this->file.is_open()){
        if(DEBUG == true)
            std::cout << "Inserting key " << key << std::endl;
        // Load root from BTree
        this->load_node(this->root);
        // If root node is empty
        if(this->node->is_empty()){
            this->node->keys[0] = key;  // Sets node keys
            this->node->n = 1;          // Updates node key count
            this->store_node(this->root, *this->node);

            if(DEBUG == true)
                std::cout << "Inserted on empty node" << std::endl;
        }else{
            // If root is full, then tree grows in height
            if(this->node->n == this->t){
                if(DEBUG == true)
                    std::cout << "Spliting root node" << std::endl;

                // Create new node
                BTreeNode s(this->t, false);

                // Make old root as child of new root
                s.C[0] = this->node_ptr;

                // Split the old root and move 1 key to the new root
                this->splitChild(0, this->node, &s);

                this->store_node(this->node_ptr, *this->node);
                // New root has two children now. Decide which of the
                // two children is going to have new key
                int i = 0;
                if(s.keys[0] < key)
                    i++;

                this->load_node(s.C[i]);
                this->insertNonFull(key);

                // Add new node to secondary memory
                int ptr = this->add_node(s);

                if(DEBUG == true)
                    std::cout << "New root pointer is " << ptr << std::endl;

                // Change root
                this->root = ptr;

                this->store_info_header(this->root, this->t);
            }else{
                this->insertNonFull(key);
            }
        }
    }
}

void BTree::insertNonFull(int key){
    // Initialize index as index of rightmost element
    int i = this->node->n-1;

    // If this is a leaf node
    if (this->node->leaf == true)
    {
        // The following loop does two things
        // a) Finds the location of new key to be inserted
        // b) Moves all greater keys to one place ahead
        while (i >= 0 && this->node->keys[i] > key)
        {
            this->node->keys[i+1] = this->node->keys[i];
            i--;
        }
        
        // Insert the new key at found location
        this->node->keys[i+1] = key;
        this->node->n = this->node->n+1;

        this->store_node(this->node_ptr, *this->node);
    }
    else // If this node is not leaf
    {
        BTreeNode *node_aux = new BTreeNode(0, 0);
        int node_ptr_aux = this->node_ptr;

        // There was a bug that I couldn't solve, so here is an ugly solution
        char* data = this->node->serialize();
        node_aux->deserialize(data);

        // Find the child which is going to have the new key
        while (i >= 0 && this->node->keys[i] > key){
            if(DEBUG == true)
                std::cout << i << " Key comparison x > y " << this->node->keys[i] << " " << key << std::endl;
            i--;
        }

        int next_node = node_aux->C[i+1];
        
        this->load_node(next_node); 

        // See if the found child is full
        if (this->node->n == this->t){
            if(DEBUG == true)
                std::cout << "Spliting leaf node with pointer " << this->node_ptr << std::endl;            

            std::cout << this->node->keys[0] << std::endl;

            // If the child is full, then split it
            this->splitChild(i+1, this->node, node_aux);

            // After split, the middle key of C[i] goes up and
            // C[i] is splitted into two.  See which of the two
            // is going to have the new key
            std::cout << node_aux->keys[i+1] << " " << key << std::endl;
            if (node_aux->keys[i+1] < key)
                i++;

            this->store_node(node_ptr_aux, *node_aux);
        }

        this->store_node(this->node_ptr, *this->node);

        this->load_node(node_aux->C[i+1]);

        this->insertNonFull(key);
    }

}

void BTree::splitChild(int i, BTreeNode *y, BTreeNode *p)
{
    // Create a new node which is going to store (t-1) keys
    // of y
    BTreeNode z = BTreeNode(y->t, y->leaf);
    z.n = this->t/2;

    // Copy the last (t-1) keys of y to z
    for (int j = 0; j < this->t/2; j++)
        z.keys[j] = y->keys[j+t-t/2];

    // Copy the last t children of y to z
    if (y->leaf == false)
    {
        for (int j = 0; j < t/2+1; j++)
            z.C[j] = y->C[j+t-t/2];
    }

    // Reduce the number of keys in y
    y->n = t-t/2-1;

    // Since this node is going to have a new child,
    // create space of new child
    for (int j = p->n; j >= i+1; j--)
        p->C[j+1] = p->C[j];

    int ptr = this->add_node(z);

    // Link the new child to this node
    p->C[i+1] = ptr;

    // A key of y will move to this node. Find location of
    // new key and move all greater keys one space ahead
    for (int j = p->n-1; j >= i; j--)
        p->keys[j+1] = p->keys[j];

    // Copy the middle key of y to this node
    p->keys[i] = y->keys[t-t/2-1];

    // Increment count of keys in this node
    p->n = p->n + 1;
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
