#pragma once
#include <iostream>
#include <fstream>
#include <cstring>
#include <climits>
#include <queue>
#include <sstream>
#include <vector>
using namespace std;
const int BLOCK_SIZE = 4096;
const int MIN_DEGREE = 3;
const int MAX_KEYS = 2 * MIN_DEGREE - 1;
const int MIN_KEYS = MIN_DEGREE - 1;

#pragma pack(push,1)
struct SuperBlock {
    int root_block;
    int total_blocks;
    char bitmap[BLOCK_SIZE - 8];
};
#pragma pack(pop)

class BTreeNode {
public:
    bool is_leaf;
    int key_count;
    int disk_index;

    int keys[MAX_KEYS];
    int children[MAX_KEYS + 1];           // disk indices
    BTreeNode* mem_children[MAX_KEYS + 1]; // in-memory pointers

    BTreeNode(bool leaf = true) {
        is_leaf = leaf;
        key_count = 0;
        disk_index = -1;
        for (int i = 0; i < MAX_KEYS; ++i) keys[i] = INT_MIN;
        for (int i = 0; i <= MAX_KEYS; ++i) children[i] = -1;
        for (int i = 0; i <= MAX_KEYS; ++i) mem_children[i] = nullptr;
    }
};

class PersistentBTree {
private:
    string filename;
    fstream file;
    SuperBlock superblock;
    BTreeNode* root;

    // ---------- Bitmap helpers ----------
    inline void set_bitmap_bit(int idx, bool val) {
        int byte_idx = idx / 8;
        int bit_idx = idx % 8;
        unsigned char& b = reinterpret_cast<unsigned char&>(superblock.bitmap[byte_idx]);
        if (val) b |= (1 << bit_idx);
        else     b &= ~(1 << bit_idx);
    }

    inline bool get_bitmap_bit(int idx) const {
        int byte_idx = idx / 8;
        int bit_idx = idx % 8;
        unsigned char b = (unsigned char)superblock.bitmap[byte_idx];
        return (b >> bit_idx) & 1;
    }

    int find_free_block() {
        // block 0 reserved for superblock
        for (int i = 1; i < superblock.total_blocks; ++i)
            if (!get_bitmap_bit(i)) return i;
        return -1;
    }

    void write_superblock() {
        file.seekp(0, ios::beg);
        file.write(reinterpret_cast<char*>(&superblock), BLOCK_SIZE);
        file.flush();
    }

    // ---------- Serialization ----------
    void serialize_node(const BTreeNode* node, char* buffer) {
        memset(buffer, 0, BLOCK_SIZE);
        int offset = 0;

        memcpy(buffer + offset, &node->is_leaf, sizeof(node->is_leaf));
        offset += sizeof(node->is_leaf);

        memcpy(buffer + offset, &node->key_count, sizeof(node->key_count));
        offset += sizeof(node->key_count);

        memcpy(buffer + offset, &node->disk_index, sizeof(node->disk_index));
        offset += sizeof(node->disk_index);

        for (int i = 0; i < MAX_KEYS; ++i) {
            memcpy(buffer + offset, &node->keys[i], sizeof(int));
            offset += sizeof(int);
        }
        for (int i = 0; i <= MAX_KEYS; ++i) {
            memcpy(buffer + offset, &node->children[i], sizeof(int));
            offset += sizeof(int);
        }
        // rest of block unused
    }

    void deserialize_node(BTreeNode* node, const char* buffer, int block_index) {
        int offset = 0;

        memcpy(&node->is_leaf, buffer + offset, sizeof(node->is_leaf));
        offset += sizeof(node->is_leaf);

        memcpy(&node->key_count, buffer + offset, sizeof(node->key_count));
        offset += sizeof(node->key_count);

        memcpy(&node->disk_index, buffer + offset, sizeof(node->disk_index));
        offset += sizeof(node->disk_index);

        for (int i = 0; i < MAX_KEYS; ++i) {
            memcpy(&node->keys[i], buffer + offset, sizeof(int));
            offset += sizeof(int);
        }
        for (int i = 0; i <= MAX_KEYS; ++i) {
            memcpy(&node->children[i], buffer + offset, sizeof(int));
            offset += sizeof(int);
        }

        for (int i = 0; i <= MAX_KEYS; ++i) node->mem_children[i] = nullptr;
        node->disk_index = block_index;
    }

    void write_node(BTreeNode* node) {
        if (node->disk_index < 0) return;
        char buffer[BLOCK_SIZE];
        serialize_node(node, buffer);
        file.seekp((long long)node->disk_index * BLOCK_SIZE, ios::beg);
        file.write(buffer, BLOCK_SIZE);
        file.flush();
    }

    BTreeNode* read_node(int block) {
        if (block < 0) return nullptr;
        char buffer[BLOCK_SIZE];
        file.seekg((long long)block * BLOCK_SIZE, ios::beg);
        file.read(buffer, BLOCK_SIZE);
        if (!file.good()) return nullptr;

        BTreeNode* node = new BTreeNode();
        deserialize_node(node, buffer, block);
        return node;
    }

    int allocate_block_for_node(BTreeNode* node) {
        int b = find_free_block();

        if (b == -1) {
            // expand file: add 1024 new blocks
            int old = superblock.total_blocks;
            superblock.total_blocks += 1024;
            write_superblock();
            b = find_free_block();
            if (b == -1) return -1;
        }

        set_bitmap_bit(b, true);
        write_superblock();

        node->disk_index = b;
        write_node(node);
        return b;
    }

    void deallocate_block(int block_index) {
        if (block_index <= 0) return; // don't deallocate superblock
        set_bitmap_bit(block_index, false);
        write_superblock();
    }

    // ---------- B-Tree helpers ----------
    void split_child(BTreeNode* parent, int index) {
        BTreeNode* y = parent->mem_children[index];
        if (!y) return;

        BTreeNode* z = new BTreeNode(y->is_leaf);
        z->key_count = MIN_DEGREE - 1;

        // move last t-1 keys of y to z
        for (int j = 0; j < MIN_DEGREE - 1; ++j) {
            z->keys[j] = y->keys[j + MIN_DEGREE];
            y->keys[j + MIN_DEGREE] = INT_MIN;
        }

        // move children if not leaf
        if (!y->is_leaf) {
            for (int j = 0; j < MIN_DEGREE; ++j) {
                z->children[j] = y->children[j + MIN_DEGREE];
                z->mem_children[j] = y->mem_children[j + MIN_DEGREE];
                y->children[j + MIN_DEGREE] = -1;
                y->mem_children[j + MIN_DEGREE] = nullptr;
            }
        }

        y->key_count = MIN_DEGREE - 1;

        // shift parent's children/pointers to make room
        for (int j = parent->key_count; j >= index + 1; --j) {
            parent->children[j + 1] = parent->children[j];
            parent->mem_children[j + 1] = parent->mem_children[j];
        }

        // allocate disk block for z and update parent
        allocate_block_for_node(z);
        parent->children[index + 1] = z->disk_index;
        parent->mem_children[index + 1] = z;

        // shift parent's keys
        for (int j = parent->key_count - 1; j >= index; --j)
            parent->keys[j + 1] = parent->keys[j];

        // move median key up
        parent->keys[index] = y->keys[MIN_DEGREE - 1];
        y->keys[MIN_DEGREE - 1] = INT_MIN;

        parent->key_count++;

        // write nodes to disk
        write_node(y);
        write_node(z);
        write_node(parent);
    }

    void insert_non_full(BTreeNode* node, int k) {
        // Check for duplicate in this node
        for (int i = 0; i < node->key_count; ++i) {
            if (node->keys[i] == k) {
                cout << "ERROR: Duplicate key " << k << " detected during insertion.\n";
                return;
            }
        }

        int i = node->key_count - 1;

        if (node->is_leaf) {
            // shift and insert
            while (i >= 0 && node->keys[i] > k) {
                node->keys[i + 1] = node->keys[i];
                i--;
            }

            // Double-check we're not inserting a duplicate
            if (i >= 0 && node->keys[i] == k) {
                cout << "ERROR: Duplicate key " << k << " detected during insertion.\n";
                return;
            }

            node->keys[i + 1] = k;
            node->key_count++;
            write_node(node);
        }
        else {
            while (i >= 0 && node->keys[i] > k) i--;
            i++;

            // ensure child loaded
            if (!node->mem_children[i] && node->children[i] != -1)
                node->mem_children[i] = read_node(node->children[i]);

            if (!node->mem_children[i]) {
                // child doesn't exist (shouldn't happen in proper tree) -> create
                node->mem_children[i] = new BTreeNode(true);
                allocate_block_for_node(node->mem_children[i]);
                node->children[i] = node->mem_children[i]->disk_index;
                write_node(node);
            }

            if (node->mem_children[i]->key_count == MAX_KEYS) {
                split_child(node, i);
                if (k > node->keys[i]) i++;
            }

            if (!node->mem_children[i] && node->children[i] != -1)
                node->mem_children[i] = read_node(node->children[i]);

            insert_non_full(node->mem_children[i], k);
        }
    }

    // ---------- Deletion Implementation ----------
    int find_key(BTreeNode* node, int k) {
        int idx = 0;
        while (idx < node->key_count && node->keys[idx] < k)
            ++idx;
        return idx;
    }

    void remove_from_leaf(BTreeNode* node, int idx) {
        for (int i = idx + 1; i < node->key_count; ++i)
            node->keys[i - 1] = node->keys[i];
        node->key_count--;
        node->keys[node->key_count] = INT_MIN;  // Clear the last key

        write_node(node);
    }

    void remove_from_non_leaf(BTreeNode* node, int idx) {
        int k = node->keys[idx];

        // Load left child if not in memory
        if (!node->mem_children[idx] && node->children[idx] != -1)
            node->mem_children[idx] = read_node(node->children[idx]);
        BTreeNode* left_child = node->mem_children[idx];

        // Load right child if not in memory  
        if (!node->mem_children[idx + 1] && node->children[idx + 1] != -1)
            node->mem_children[idx + 1] = read_node(node->children[idx + 1]);
        BTreeNode* right_child = node->mem_children[idx + 1];

        if (left_child && left_child->key_count >= MIN_DEGREE) {
            // Case 2a: predecessor
            BTreeNode* current = left_child;
            while (!current->is_leaf) {
                int last_idx = current->key_count;
                if (!current->mem_children[last_idx] && current->children[last_idx] != -1)
                    current->mem_children[last_idx] = read_node(current->children[last_idx]);
                current = current->mem_children[last_idx];
            }
            int pred = current->keys[current->key_count - 1];
            node->keys[idx] = pred;
            write_node(node);
            remove_rec(left_child, pred);
        }
        else if (right_child && right_child->key_count >= MIN_DEGREE) {
            // Case 2b: successor
            BTreeNode* current = right_child;
            while (!current->is_leaf) {
                if (!current->mem_children[0] && current->children[0] != -1)
                    current->mem_children[0] = read_node(current->children[0]);
                current = current->mem_children[0];
            }
            int succ = current->keys[0];
            node->keys[idx] = succ;
            write_node(node);
            remove_rec(right_child, succ);
        }
        else {
            // Case 2c: merge
            merge_nodes(node, idx);

            // After merge, the key to delete is now in the merged child
            // The merged child is at the same index after merge
            if (!node->mem_children[idx] && node->children[idx] != -1)
                node->mem_children[idx] = read_node(node->children[idx]);

            if (node->mem_children[idx]) {
                remove_rec(node->mem_children[idx], k);
            }
        }
    }

    void borrow_from_prev(BTreeNode* node, int idx) {
        BTreeNode* child = node->mem_children[idx];
        BTreeNode* sibling = node->mem_children[idx - 1];

        // Shift child keys right
        for (int i = child->key_count - 1; i >= 0; --i)
            child->keys[i + 1] = child->keys[i];

        // Shift child pointers right if not leaf
        if (!child->is_leaf) {
            for (int i = child->key_count; i >= 0; --i) {
                child->children[i + 1] = child->children[i];
                child->mem_children[i + 1] = child->mem_children[i];
            }
        }

        // Move key from node to child
        child->keys[0] = node->keys[idx - 1];

        // Move last key from sibling to node
        if (!sibling->is_leaf) {
            child->children[0] = sibling->children[sibling->key_count];
            child->mem_children[0] = sibling->mem_children[sibling->key_count];
        }

        node->keys[idx - 1] = sibling->keys[sibling->key_count - 1];
        child->key_count++;
        sibling->key_count--;

        write_node(child);
        write_node(sibling);
        write_node(node);
    }

    void borrow_from_next(BTreeNode* node, int idx) {
        BTreeNode* child = node->mem_children[idx];
        BTreeNode* sibling = node->mem_children[idx + 1];

        // Move key from node to child
        child->keys[child->key_count] = node->keys[idx];
        child->key_count++;

        // Move first key from sibling to node
        node->keys[idx] = sibling->keys[0];

        // Shift sibling keys left
        for (int i = 1; i < sibling->key_count; ++i)
            sibling->keys[i - 1] = sibling->keys[i];

        // Shift sibling pointers left if not leaf
        if (!sibling->is_leaf) {
            child->children[child->key_count] = sibling->children[0];
            child->mem_children[child->key_count] = sibling->mem_children[0];

            for (int i = 1; i <= sibling->key_count; ++i) {
                sibling->children[i - 1] = sibling->children[i];
                sibling->mem_children[i - 1] = sibling->mem_children[i];
            }
        }

        sibling->key_count--;

        write_node(child);
        write_node(sibling);
        write_node(node);
    }

    void merge_nodes(BTreeNode* node, int idx) {
        BTreeNode* child = node->mem_children[idx];
        BTreeNode* sibling = node->mem_children[idx + 1];

        // Move key from parent down to child
        child->keys[MIN_DEGREE - 1] = node->keys[idx];

        // Copy keys from sibling to child
        for (int i = 0; i < sibling->key_count; ++i)
            child->keys[i + MIN_DEGREE] = sibling->keys[i];

        // Copy pointers from sibling to child if not leaf
        if (!child->is_leaf) {
            for (int i = 0; i <= sibling->key_count; ++i) {
                child->children[i + MIN_DEGREE] = sibling->children[i];
                child->mem_children[i + MIN_DEGREE] = sibling->mem_children[i];
            }
        }

        // Shift parent keys and pointers left
        for (int i = idx + 1; i < node->key_count; ++i)
            node->keys[i - 1] = node->keys[i];
        for (int i = idx + 2; i <= node->key_count; ++i) {
            node->children[i - 1] = node->children[i];
            node->mem_children[i - 1] = node->mem_children[i];
        }

        // Clear the last pointers in parent
        node->children[node->key_count] = -1;
        node->mem_children[node->key_count] = nullptr;

        node->key_count--;
        child->key_count += sibling->key_count + 1; // +1 for the parent key

        // Clear sibling's keys and pointers (important!)
        for (int i = 0; i < sibling->key_count; ++i)
            sibling->keys[i] = INT_MIN;
        for (int i = 0; i <= sibling->key_count; ++i) {
            sibling->children[i] = -1;
            sibling->mem_children[i] = nullptr;
        }

        // Deallocate sibling block
        deallocate_block(sibling->disk_index);
        delete sibling;
        node->mem_children[idx + 1] = nullptr;

        write_node(child);
        write_node(node);
    }


    void fill_child(BTreeNode* node, int idx) {
        // Load child if not in memory
        if (!node->mem_children[idx] && node->children[idx] != -1)
            node->mem_children[idx] = read_node(node->children[idx]);
        BTreeNode* child = node->mem_children[idx];

        if (!child) return;

        if (idx > 0) {
            // Load left sibling if not in memory
            if (!node->mem_children[idx - 1] && node->children[idx - 1] != -1)
                node->mem_children[idx - 1] = read_node(node->children[idx - 1]);
            BTreeNode* left_sibling = node->mem_children[idx - 1];

            if (left_sibling && left_sibling->key_count >= MIN_DEGREE) {
                borrow_from_prev(node, idx);
                return;
            }
        }

        if (idx < node->key_count) {
            // Load right sibling if not in memory
            if (!node->mem_children[idx + 1] && node->children[idx + 1] != -1)
                node->mem_children[idx + 1] = read_node(node->children[idx + 1]);
            BTreeNode* right_sibling = node->mem_children[idx + 1];

            if (right_sibling && right_sibling->key_count >= MIN_DEGREE) {
                borrow_from_next(node, idx);
                return;
            }
        }

        // Merge with appropriate sibling
        if (idx > 0 && idx <= node->key_count) {
            // Can merge with left sibling
            merge_nodes(node, idx - 1);
        }
        else if (idx < node->key_count) {
            // Can merge with right sibling  
            merge_nodes(node, idx);
        }
        // Note: If we reach here and can't merge, the tree is in an invalid state
        // but this shouldn't happen in a proper B-tree
    }

    void remove_rec(BTreeNode* node, int k) {
        if (!node) return;

        int idx = find_key(node, k);

        // Key found in this node
        if (idx < node->key_count && node->keys[idx] == k) {
            if (node->is_leaf) {
                remove_from_leaf(node, idx);
            }
            else {
                remove_from_non_leaf(node, idx);
            }
        }
        else {
            if (node->is_leaf) return; // Key not found

            // Load child if not in memory
            if (!node->mem_children[idx] && node->children[idx] != -1)
                node->mem_children[idx] = read_node(node->children[idx]);
            BTreeNode* child = node->mem_children[idx];

            if (!child) return;

            if (child->key_count < MIN_DEGREE) {
                fill_child(node, idx);
                // After filling, we need to reload child pointer as it might have changed
                if (!node->mem_children[idx] && node->children[idx] != -1)
                    node->mem_children[idx] = read_node(node->children[idx]);
                child = node->mem_children[idx];
            }

            // Determine which child to traverse to
            if (idx > node->key_count) {
                remove_rec(node->mem_children[idx - 1], k);
            }
            else {
                remove_rec(child, k);
            }
        }
    }

    void cleanup_mem(BTreeNode* node) {
        if (!node) return;
        for (int i = 0; i <= node->key_count; ++i) {
            if (node->mem_children[i]) {
                cleanup_mem(node->mem_children[i]);
                delete node->mem_children[i];
                node->mem_children[i] = nullptr;
            }
        }
    }

public:
    PersistentBTree(const string& fname) : filename(fname), root(nullptr) {
        bool file_exists = false;
        {
            ifstream f(fname, ios::binary);
            file_exists = f.good();
        }

        // open file read/write, create if missing
        file.open(fname, ios::in | ios::out | ios::binary);
        if (!file.is_open() || !file_exists) {
            // create new file and write initial superblock
            file.open(fname, ios::out | ios::binary);
            file.close();
            file.open(fname, ios::in | ios::out | ios::binary);
            initialize_superblock();
        }

        // try read superblock
        file.seekg(0, ios::beg);
        file.read(reinterpret_cast<char*>(&superblock), BLOCK_SIZE);

        if (!file.good() || superblock.total_blocks < 1 || superblock.root_block < 0) {
            initialize_superblock();
            // create empty root
            root = new BTreeNode(true);
            allocate_block_for_node(root);
            superblock.root_block = root->disk_index;
            write_superblock();
        }
        else {
            // load root node from disk
            if (superblock.root_block >= 0) {
                root = read_node(superblock.root_block);
            }
            else {
                // fallback: create new root
                root = new BTreeNode(true);
                allocate_block_for_node(root);
                superblock.root_block = root->disk_index;
                write_superblock();
            }
        }
    }

    ~PersistentBTree() {
        if (root) {
            write_node(root);
            cleanup_mem(root);
            delete root;
            root = nullptr;
        }
        write_superblock();
        if (file.is_open()) file.close();
    }

    void initialize_superblock() {
        superblock.root_block = -1;
        superblock.total_blocks = 8192; // initial size
        memset(superblock.bitmap, 0, sizeof(superblock.bitmap));
        // reserve block 0 for superblock
        set_bitmap_bit(0, true);
        // write to file
        write_superblock();
    }

    void insert(int key) {
        // Check if key already exists
        if (search(key).first) {
            cout << "ERROR: Key " << key << " already exists in B-tree.\n";
            return;
        }

        if (!root) {
            root = new BTreeNode(true);
            allocate_block_for_node(root);
            superblock.root_block = root->disk_index;
            write_superblock();
        }

        if (root->key_count == MAX_KEYS) {
            BTreeNode* s = new BTreeNode(false);
            allocate_block_for_node(s);
            s->children[0] = root->disk_index;
            s->mem_children[0] = root;
            superblock.root_block = s->disk_index;
            write_superblock();
            split_child(s, 0);
            root = s;
        }

        insert_non_full(root, key);
        cout << "Inserted value " << key << " into B-tree.\n";
    }

    void remove(int key) {
        if (!root) return;

        remove_rec(root, key);

        // If root becomes empty after deletion
        if (root->key_count == 0) {
            BTreeNode* old_root = root;
            if (!root->is_leaf) {
                // Make first child the new root
                if (root->children[0] != -1) {
                    root = read_node(root->children[0]);
                    superblock.root_block = root->disk_index;
                    write_superblock();
                }
                else {
                    root = nullptr;
                    superblock.root_block = -1;
                    write_superblock();
                }
            }
            else {
                // Root is leaf and empty - tree is empty
                root = nullptr;
                superblock.root_block = -1;
                write_superblock();
            }
            deallocate_block(old_root->disk_index);
            delete old_root;
        }
    }

    pair<bool, vector<int>> search(int key) {
        vector<int> path;
        bool found = search_rec(root, key, path);
        return { found, path };
    }

    bool search_rec(BTreeNode* node, int key, vector<int>& path) {
        if (!node) return false;
        path.push_back(node->disk_index);

        int i = 0;
        while (i < node->key_count && key > node->keys[i]) i++;

        if (i < node->key_count && node->keys[i] == key) return true;
        if (node->is_leaf) return false;

        if (!node->mem_children[i] && node->children[i] != -1)
            node->mem_children[i] = read_node(node->children[i]);

        return search_rec(node->mem_children[i], key, path);
    }

    void print_tree() {
        if (!root) { cout << "Tree empty\n"; return; }

        queue<pair<BTreeNode*, int>> q;
        q.push({ root, 0 });
        int level = -1;

        while (!q.empty()) {
            auto pr = q.front(); q.pop();
            BTreeNode* node = pr.first;
            int lvl = pr.second;

            if (lvl != level) {
                level = lvl;
                cout << "\nLevel " << level << ": ";
            }

            cout << "[B" << node->disk_index << ": ";
            for (int i = 0; i < node->key_count; ++i) {
                cout << node->keys[i];
                if (i + 1 < node->key_count) cout << ",";
            }
            cout << "] ";

            if (!node->is_leaf) {
                for (int i = 0; i <= node->key_count; ++i) {
                    if (!node->mem_children[i] && node->children[i] != -1)
                        node->mem_children[i] = read_node(node->children[i]);
                    if (node->mem_children[i])
                        q.push({ node->mem_children[i], lvl + 1 });
                }
            }
        }
        cout << "\n";
    }
};