#include<stdio.h>
#include <algorithm>
#include <cstddef>
#include <utility>
#include<stdexcept>
#include<iostream>
#include<map>
#include <assert.h>
using namespace std;
//=======================================


/*
	M阶B树 满足性质
	每个节点至多拥有M棵子树
	根节点至少拥有两颗子树
	除了根节点外，其他节点分支节点至少拥有M/2棵子树
	所有节点都在同一层
	有k棵子树的分支节点存在k-1个关键字，关键字按照递增顺序进行排序
	关键字数量满足ceil（M/2）-1 <= n <= M-1

*/
#define DEGREE		4

enum{
	//节点枚举，
	Node_Normal = 0,//非叶子节点
	Node_Leaf ,	//叶子节点
};

typedef int KEY_VALUE;

typedef struct _btree_node {
	KEY_VALUE *keys;				// 节点上存放的key列表
	struct _btree_node **childrens; //节点执行子节点的列表
	int num;						//节点现有key的数量
	int leaf;						//节点是否为叶子节点
} btree_node;

//B树结构体 包含root节点 与此树的度
typedef struct _btree {
	btree_node *root; 
	int degree;
} btree;

/*
	degree 树的度
	leaf  是否为子节点，1：是， 0：不是
*/
btree_node *btree_create_node(int degree, int leaf) {
	//创建一个节点，初始化相关数据，最多包含degree-1个关键字，degree个子节点，分裂的最大临界值
	btree_node *node = (btree_node*)calloc(1, sizeof(btree_node));
	if (node == NULL) assert(0);

	node->leaf = leaf;
	node->keys = (KEY_VALUE*)calloc(1, (degree-1)*sizeof(KEY_VALUE));
	node->childrens = (btree_node**)calloc(1, degree * sizeof(btree_node));
	node->num = 0;

	return node;
}


void btree_create(btree *T, int degree) {
	T->degree = degree;
	
	btree_node *x = btree_create_node(degree, 1);
	T->root = x;
	
}

/*
	i 节点的子节点编号，要分裂的节点相对于父节点编号
*/
void btree_split_child(btree *T, btree_node *x, int number) {
	int degree = T->degree;

	//要分裂的节点将从split_index开始移到同级节点中
	int split_index = degree/2;
	printf("结点开始进行分裂 i的值为 %d... ",number);
	//y是要分裂的节点，z是y分裂后同级别的节点
	btree_node *y = x->childrens[number];
	btree_node *z = btree_create_node(degree, y->leaf);


	//将z节点中分裂数据后面的数据移动到新的节点y中
	int j = 0,k=0;
	for (j = split_index;j < y->num;j ++) {
		printf("原结点 %c 放到新节点坐标%d上 ...\n",y->keys[j], k);
		z->keys[k++] = y->keys[j];
		z->num++;
	}
	printf("\n");


	//如果y不是叶子节点 分裂时所属子节点也要分
	if (y->leaf == 0) {
			int index = 0;
		for (j = split_index;j < T->degree;j ++) {
			printf("不是叶子节点 原结点 %c 值放到新节点%d坐标上 ... \n",y->keys[j+degree/2], j);
			z->childrens[index++] = y->childrens[split_index];
		}
	}

	y->num = split_index;

	//节点中指向子节点列表处理 按顺序后移 ， children指针变化，
	for (j = x->num;j > number;j --) {
		printf("子节点后移  ... \n");
		x->childrens[j+1] = x->childrens[j];

	}
	x->childrens[number+1] = z;


	//节点中数据处理 找地方插入
	for (j = x->num-1;j >= number;j --) {
		printf("x节点 给新节点找位置 原结点 %c 值后移 ... \n",x->keys[j]);
		x->keys[j+1] = x->keys[j];
	}
	printf("节点 %d 位置 放入key 值 %c  ... \n",x,y->keys[split_index-1]);
	x->keys[number] = y->keys[split_index-1];
	x->num += 1;


	//分裂后子节点一个数据移动到父节点了 子节点数量减1
	y->num --;
	
}

void btree_insert_nonfull(btree *T, btree_node *x, KEY_VALUE k) {


	//i 理解为节点中坐标值
	int i = x->num - 1;
	
	if (x->leaf == 1) {//叶子节点插入
		//下标从大到小获取key与插入的key比较，如果比插入的key大，则后移（类似插入排序）
		while (i >= 0 && x->keys[i] > k) {
			printf("     叶子节点 %d 坐标值大 %c 后移... \n",i, x->keys[i]);
			x->keys[i+1] = x->keys[i];
			i --;
		}
		//将key放到相应位置，节点包含数量+1
		printf("        叶子结点 %d 坐标放入值 %c ... \n",i+1, k);
		x->keys[i+1] = k;
		x->num += 1;
		
	} else {
		
		while (i >= 0 && x->keys[i] > k) {
			printf("非叶子结点 %d 坐标值%c 坐标前移 ... \n",i, x->keys[i]);
			i --;
		}

		if (x->childrens[i+1]->num == T->degree-1) {
			printf("节点子节点%d 值达到分裂条件进行分裂 ... \n", i+1);
			btree_split_child(T, x, i+1);

			if (k > x->keys[i+1])
			{
				printf("key %c 值大于节点最后一个位置的值 %c 后移... \n",k,x->keys[i+1]);
				i++;
			}
		}
		printf("子节点 %d 插入新节点 %c ... \n",i+1, k);
		btree_insert_nonfull(T, x->childrens[i+1], k);
	}
}

void btree_insert(btree *T, KEY_VALUE key) {

	btree_node *r = T->root;
	if (r->num ==  T->degree - 1) {//节点包含数量满了，需要进行分裂操作
		
		//分裂是创建一个父节点，要分裂的节点作为父节点的第一个孩子节点
		btree_node *node = btree_create_node(T->degree, 0);
		T->root = node;

		node->childrens[0] = r;
		
		btree_split_child(T, node, 0);

		//此时 分裂后 父节点包含一个数据，比较此数据与插入的数据大小，确定应该插入到的父节点的子节点编号
		int i = 0;
		if (node->keys[0] < key)
		 i++;

		btree_insert_nonfull(T, node->childrens[i], key);
		
	} else {
		//节点数量没有满，继续直接插入操作
		printf("插入 节点 %c ...", key);
		btree_insert_nonfull(T, r, key);
	}
}


int main(){

	btree T = {0};

	btree_create(&T, DEGREE);
	srand(48);

	int i = 0;
	char key[42] = "aBcDhFjHmJkLlCuEgGqIwKeMrQtAyRfSTUVWXY";
	for (i = 0;i < 32;i ++) {
		//key[i] = rand() % 1000;
		//printf("新节点 %c ..", key[i]);
		btree_insert(&T, key[i]);
		//printf("节点 %c ..完成\n\n", key[i]);
	}

	return 0;
}
