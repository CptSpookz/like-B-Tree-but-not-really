#include "b_tree_file.hh"

int main(){
    // BTree file test
    BTree btree = BTree("btree");

    btree.init(63);

    btree.load_info_header();

    return 0;
}
