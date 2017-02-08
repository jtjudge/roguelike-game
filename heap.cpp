#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

#include "stack.h"
#include "heap.h"

heap::h_node *create_node(void *data, heap::h_node *p) {
  heap::h_node *n = new heap::h_node();
  n->data = data;
  n->parent = p;
  n->left = 0;
  n->right = 0;
  return n;
}

void destroy_node(heap::h_node *n) {
  n->data = 0;
  n->parent = n->left = n->right = 0;
  delete n;
}

void heap::create(int (*cmp)(void *, void *),
	      void (*disp)(void *)) {
  this->min = 0;
  this->size = 0;
  this->cmp = cmp;
  this->disp = disp;
}

void dest_rec(heap::h_node *n) {
  if(!n) return;
  dest_rec(n->left);
  destroy_node(n);
  dest_rec(n->right);
}

void heap::destroy(void) {
  dest_rec(this->min);
  this->cmp = 0;
  this->disp = 0;
}

void disp_rec(heap *h, heap::h_node *n) {
  if(n == 0) return;
  disp_rec(h, n->left);
  printw("(");
  if(n == h->min) {
    printw(">>>");
  }
  h->disp(n->data);
  if(n == h->min) {
    printw("<<<");
  }
  printw(") ");
  disp_rec(h, n->right);
}

void heap::display(void) {
  int i;
  for(i = 0; i < 80; i++) {
    mvaddch(24, i, ' ');
  }
  move(24, 0);
  disp_rec(this, this->min);
}

void swap_node_data(heap::h_node *n, heap::h_node *m) {
  void *tmp = n->data;
  n->data = m->data;
  m->data = tmp;
}

void heap::insert(void *data) {
  if(this->size == 0) {
    this->min = create_node(data, 0);
    this->size++;
    return;
  }
  c_stack *s = new c_stack();
  s->create(this->disp);
  int current = this->size + 1;
  int left = 0, right = 1;
  while(current > 1) {
    if(current % 2 == 1) {
      s->push((void *)&right);
    } else {
      s->push((void *)&left);
    }
    current /= 2;
  }
  heap::h_node *n = this->min;
  heap::h_node *m = 0;
  while(s->size > 0) {
    int *dir = (int *)s->pop();
    if(s->size == 0) {
      m = create_node(data, n);
      if(*dir == left) {
	n->left = m;
      } else {
	n->right = m;
      }
    } else {
      if(*dir == left) {
	n = n->left;
      } else {
	n = n->right;
      }
    }
  }
  s->destroy();
  delete s;
  this->size++;
  while(m->parent && this->cmp(m->data, m->parent->data) < 0) {
    swap_node_data(m, m->parent);
    m = m->parent;
  }
}

heap::h_node *get_min_child(heap *h, heap::h_node *n) {
  if(!n) return 0;
  if(n->left && n->right) {
    return h->cmp(n->left->data, n->right->data) > 0 ? n->right : n->left;
  } else if(n->left) {
    return n->left;
  } else if(n->right) {
    return n->right;
  } else {
    return 0;
  }
}

void *heap::remove_min(void) {
  if(this->size == 0) return NULL;
  c_stack *s = new c_stack();
  s->create(this->disp);
  int current = this->size;
  int left = 0, right = 1;
  while(current > 1) {
    if(current % 2 == 1) {
      s->push((void *)&right);
    } else {
      s->push((void *)&left);
    }
    current /= 2;
  }
  heap::h_node *n = this->min;
  while(s->size > 0) {
    int *dir = (int *)s->pop();
    if(*dir == left) {
      n = n->left;
    } else {
      n = n->right;
    }
  }
  s->destroy();
  delete s;
  /* n now points to lowest leaf in tree */
  void *min_data = this->min->data;
  swap_node_data(n, this->min);
  if(n->parent) {
    if(n == n->parent->right) {
      n->parent->right = 0;
    } else {
      n->parent->left = 0;
    }
  }
  destroy_node(n);
  this->size--;
  if(this->size > 0) {
    n = this->min;
    heap::h_node *m = get_min_child(this, n);
    while(m && this->cmp(m->data, n->data) < 0) {
      swap_node_data(m, n);
      n = m;
      m = get_min_child(this, n);
    }
  } else {
    this->min = 0;
  }
  return min_data;
}
