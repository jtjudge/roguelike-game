#ifndef HEAP_H
# define HEAP_H

class heap {
 public:
  class h_node {
  public:
    h_node *parent;
    h_node *left;
    h_node *right;
    void *data;
    h_node() {}
    ~h_node() {}
  };
  h_node *min;
  int size;
  int (*cmp)(void *, void *);
  void (*disp)(void *);
  heap() {}
  ~heap() {}
  void create(int (*cmp)(void *, void *),
	      void (*disp)(void *));
  void destroy(void);
  void display(void);
  void insert(void *data);
  void *remove_min(void);
};

#endif
