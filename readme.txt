# Algoritmos e Teoria de Grafos - Trabalho de Implementação
## Decomposição de um grafo em seus blocos (subgrafos maximais sem vértices de corte)
## Feito por Gabriel de Oliveira Pontarolo - GRR20203895

* O programa faz a utilização de 3 algoritmos principais: 
    - Low-Point juntamente com o "Corolário 74" para encontrar os vértices de corte 
    - Busca em Profundiade (DFS) para gerar o inverso da pós ordem para a Decomposição
    - Decomposição para encontrar as componentes após a retirada dos vértices de corte 
* Após isso, ele junta novamente os vértices de corte com suas respectivas componentes e imprime os blocos resultantes
* A representação de grafos para a entrada é feita em dot com o auxilio da biblioteca cgraph, porém ele a converte para uma estrutura própria que utiliza matriz de adjascência para que a manipulação pelos algoritmos seja mais simples