/*
 * Copyright (C) 2017 Simon Shields <simon@lineageos.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include "llist.h"

struct node {
	struct node *next;
	void *val;
};

node_t *listCreate(void *val) {
	node_t *head = calloc(1, sizeof(*head));
	head->next = NULL;
	head->val = val;
	return head;
}

node_t *listNext(node_t *head, node_t *cur) {
	if (cur == NULL)
		return head;
	return cur->next;
}

void *listGet(node_t *node) {
	return node->val;
}

void listAppend(node_t *head, void *what) {
	struct node *cur = head;
	struct node *new = malloc(sizeof(*new));

	while (cur->next != NULL)
		cur = cur->next;

	new->val = what;
	new->next = NULL;
	cur->next = new;
}


void listFree(node_t *head) {
	struct node *cur = head->next;
	struct node *tmp = head;
	while (cur != NULL) {
		free(tmp);
		tmp = cur;
		cur = cur->next;
	}
	free(tmp);
}
