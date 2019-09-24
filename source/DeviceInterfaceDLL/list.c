#include <windows.h>
#include <string.h>
#include "list.h"
#include "device.h"

//struct LNode L;

List ListAdd(List root, LPVOID x)
{
	
	List ls = (List)malloc(sizeof(struct LNode));
	if (ls == NULL)return NULL;
	ls->data = x;
	ls->next = root;
	return ls;
}

List ListClear(List root)
{
	List ls;
	while (root != NULL)
	{
		ls = root->next;
		free(root);
		root = ls;
	}
	return NULL;
}

List ListRemove(List root, List ls)
{
	if (root == NULL)return NULL;

	if (root == ls)
	{
		root = ls->next;
		free(ls);
		return NULL;
	}
	else
	{
		List pre = root;
		while (pre->next != NULL)
		{
			if (pre->next == ls)
			{
				pre->next = ls->next;
				free(ls);
				return root;
			}
		}
	}
	return root;
}




