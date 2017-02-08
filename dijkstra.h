#ifndef DIJKSTRA_H
# define DIJKSTRA_H

int cmp_cell(void *c1, void *c2);

int t_cmp_cell(void *c1, void *c2);

void dijkstra(cell *target, int tunnel);

#endif
