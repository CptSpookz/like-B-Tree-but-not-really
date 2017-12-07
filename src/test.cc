#include "b_tree_file.hh"

int main(){
    // BTree file test
    BTree btree = BTree("btree");

    btree.init(3);

    btree.load_info_header();

<<<<<<< HEAD
    btree.insert(4000);
    btree.insert(4040);
    btree.insert(3902);
=======
    btree.insert(32);

    btree.insert(16);

    btree.insert(48);

    btree.insert(128);

    btree.insert(8);

    btree.insert(80);

    btree.insert(96);
    
    btree.insert(64);

    btree.insert(112);
>>>>>>> f2929a0c842f8bbb5861f385d94bbc90b19b6231

    btree.search(4000);
    btree.search(3902);
    return 0;
}
