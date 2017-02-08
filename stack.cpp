#include <stdio.h>
#include <stdlib.h>

#include "stack.h"

c_stack::s_node *create_node(void *data, c_stack::s_node *next) {
  c_stack::s_node *n = new c_stack::s_node();
  n->data = data;
  n->next = next;
  return n;
}

void destroy_node(c_stack::s_node *n) {
  n->data = 0;
  n->next = 0;
  delete n;
}

void c_stack::create(void (*disp)(void *)) {
  this->top = 0;
  this->size = 0;
  this->disp = disp;
}

void c_stack::destroy(void) {
  c_stack::s_node *tmp;
  while(this->top) {
    tmp = this->top;
    this->top = this->top->next;
    destroy_node(tmp);
  }
  this->top = 0;
  this->disp = 0;
}

void c_stack::display(void) {
  c_stack::s_node *n = this->top;
  while(n) {
    this->disp(n->data);
    printf(" ");
    n = n->next;
  }
  printf("\n");
}

void c_stack::push(void *data) {
  c_stack::s_node *n = create_node(data, this->top);
  this->top = n;
  this->size++;
}

void *c_stack::pop(void) {
  void *data = this->top->data;
  c_stack::s_node *tmp = this->top;
  this->top = this->top->next;
  destroy_node(tmp);
  this->size--;
  return data;
}
