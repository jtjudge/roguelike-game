#ifndef STACK_H
# define STACK_H

class c_stack {
 public:
  class s_node {
  public:
    s_node *next;
    void *data;
    s_node() {};
    ~s_node() {};
  };
  s_node *top;
  int size;
  void (*disp)(void *);
  void create(void (*disp)(void *));
  void destroy(void);
  void push(void *data);
  void *pop(void);
  void display(void);
};

#endif
