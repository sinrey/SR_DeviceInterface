#pragma once
#include "device.h"

typedef struct LNode *List;
struct LNode {
	LPVOID data;
	List next;
};

List ListAdd(List root, LPVOID x);
List ListClear(List root);
List ListRemove(List root, List ls);
