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

    // A function to search key on tree
    BTreeNode* search(int key);
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

BTreeNode* BTreeNode::search(int k){
    // Find the first key greater than or equal to k
    int i = 0;
    while (i < this->n && k > this->keys[i]) i++;

    // If the key found is is equal to k, return this node
    if (keys[i] == k)
        return this;

    // If it isn't, return a null pointer
    return nullptr;
}

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
    
        delete[] buffer;
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

        delete[] buffer;
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
        if((size-sizeof(int)*2)/512 <= (unsigned int)ptr){
            this->node_ptr = ptr;
            char data[512];

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
        std::cout << ptr << std::endl;
        // Check if pointer is valid on file
        if((((size-sizeof(int)*2)/512)-1 >= (unsigned int)ptr)){
            char* data = node.serialize();

            if(DEBUG == true)
                std::cout << "Storing node" << std::endl;

            this->file.seekg(sizeof(int)*2+ptr*512, this->file.beg);
            this->file.write(data, 512);
        }
    }
}

int BTree::add_node(BTreeNode node){
    if(this->file.is_open()){
        char *data = node.serialize();

        this->file.seekp(0, this->file.end);
        this->file.write(data, 512);
        
        return (((unsigned long int)this->file.tellp() - (sizeof(int)*2))/512);
    }
    return -1;
}

void BTree::insert(int key){
    if(this->file.is_open()){
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
                // Create new node
                BTreeNode s(this->t, false);

                // Make old root as child of new root
                s.C[0] = this->node_ptr;

                // Split the old root and move 1 key to the new root
                // TODO: splitChild()
                
                // New root has two children now. Decide which of the
                // two children is going to have new key
                /* int i = 0;
                if(s->keys[0] < key)
                    i++;
                s.C[i].insertNonFull(key) */ 

                // Add new node to secondary memory
                int ptr = this->add_node(s);

                // Change root
                this->root = ptr;
            }
        }
    }
}

BTreeNode* BTree::search(int key){
    if(this->file.is_open()){
	// If a node is already is loaded, save it so it's not lost
	BTreeNode* result = nullptr;
	// Load the root and check if it is empty
	this->load_node(this->root);
	// If the root is not empty, begin search
	if(!this->node->is_empty()){
            result = this->node->search(key);
	    // If the result is a null pointer, load a new node until a leaf is reached
            while (!this->node->leaf && result == nullptr){
	        int i = 0;
		while (i < this->node->n && key > this->node->keys[i]) i++;
		    // Load the appropriate child node 
		    this->load_node(this->node->C[i]);

		    // Try searching again
		    result = this->node->search(key);
	    }
	}
	if(DEBUG ){
		if (result != nullptr)
			std::cout << "Found key " << std::endl;
		else
			std::cout << "Key was not found" << std::endl;
	}
	return result;
    }
    return nullptr;
}
