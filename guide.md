# Basic methods (FreshVamanaIndex)

Data is stored separately in a `_data_store` in class `FreshVamanaIndex`

The graph is stored in `_graph_store` in `FreshVamanaIndex`

## Insert

`insert` is equivalent to `insert_point` 

Line 2858 in https://github.com/microsoft/DiskANN/blob/main/src/index.cpp

1. Acquire locks
2. Update data store
3. Search and prune (GreedySearch + RobustPrune?) -> `pruned_list`
4. Update Nout(p) using the nodes from `pruned_list`
    
`prune` equivalent to `prune_neighbors (InMemoryGraph)` and `search_for_point_and_prune (FreshVamanaIndex)`.



`delete` equivalent to `lazy_delete`




Found in https://github.com/microsoft/DiskANN/blob/main/include/index.h