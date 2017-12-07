#include "b_tree_file.hh"

int main(){
    // BTree file test
    BTree btree = BTree("btree");

    btree.init(63);

    btree.load_info_header();

    btree.insert(4000);
    btree.insert(4040);
    btree.insert(3902);

    btree.search(4000);
    btree.search(3902);
    return 0;
}
