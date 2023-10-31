The num_gaphs in StreamPipeline.cpp in finalize function is set to num_gaphs-(number of empty graphs)
It also sets the _manager.num_gaphs (which is in GraphManagerPipeline)
(the reason is that in serial run the last graph notify the first grpah to run but if graphs are 6 but just 4 have nodes (when user give more layers in --order parameters, 
it should ignore those last two graphs and set the num_gaphs to 4 (so that in graph manager run function know that graph 4 is last graph)))

We ignore setup and run of the empty graphs in finalize and run functions of the streamline also(to not cause segmentation fault error)

****Later we want to redefine graphs (merge graphs that are connected and have same PE): it happens when there are two branches,
before branch is set to PE1, the first branch is set to PE2 and second branch is set to PE1 again. it consider as new graph, but we need to somehow 
manage it to not consider it as separate graphs or later merge them together.