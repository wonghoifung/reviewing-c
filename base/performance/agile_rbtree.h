#ifndef agile_rbtree_header
#define agile_rbtree_header

// reference: nginx
#include <stdint.h>
#include <stddef.h>

typedef unsigned char u_char;
typedef uintptr_t rbtree_key_t;

typedef struct rbtree_node_s rbtree_node_t;
struct rbtree_node_s {
	rbtree_key_t key;
	rbtree_node_t* left;
	rbtree_node_t* right;
	rbtree_node_t* parent;
	u_char color;
	u_char data;
};

typedef struct rbtree_s rbtree_t;
typedef void (*rbtree_insert_pt)(rbtree_node_t* root, rbtree_node_t* node, rbtree_node_t* sentinel);
struct rbtree_s {
	rbtree_node_t* root;
	rbtree_node_t* sentinel;
	rbtree_insert_pt insert;
};

#define rbtree_init(tree, s, i) \
	rbtree_sentinel_init(s); \
	(tree)->root = s; \
	(tree)->sentinel = s; \
	(tree)->insert = i

void rbtree_insert(rbtree_t* tree, rbtree_node_t* node);
void rbtree_delete(rbtree_t* tree, rbtree_node_t* node);
void rbtree_insert_node(rbtree_node_t* temp, rbtree_node_t* node, rbtree_node_t* sentinel);
rbtree_node_t* rbtree_find_key(rbtree_t* tree, rbtree_key_t key);

#define rbt_red(node) ((node)->color=1)
#define rbt_black(node) ((node)->color=0)
#define rbt_is_red(node) ((node)->color)
#define rbt_is_black(node) (!rbt_is_red(node))
#define rbt_copy_color(n1,n2) (n1->color=n2->color)

#define rbtree_sentinel_init(node) rbt_black(node)

static inline rbtree_node_t* rbtree_min(rbtree_node_t* node, rbtree_node_t* sentinel) {
	while (node->left != sentinel) {
		node = node->left;
	}
	return node;
}

void test_agile_rbtree();

#endif

