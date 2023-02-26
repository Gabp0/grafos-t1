// Algoritmos e Teoria de Grafos - Trabalho de Implementação
// Decomposição de um grafo em seus blocos (subgrafos maximais sem vértices de corte)
// Feito por Gabriel de Oliveira Pontarolo - GRR20203895
#include <graphviz/cgraph.h>
#include <stdio.h>
#include <stdlib.h>

struct vertex
// representa um vertice do grafo
{
    int index;

    int component; // variaveis para o dfs
    int pre;
    int post;
    struct vertex *father;

    int lv; // variaveis para o low point
    int low;
    int is_articulation;
    int son_num;

    char label;
    char state;
    char pad[6]; // padding para alinhar a struct na memoria
};

struct graph
// representa um grafo usando matriz de adjascencia
{
    size_t n; // numero de vertices
    size_t m; // numero de arestas
    struct vertex **vertexes;
    size_t comp_num;
    short *adj_mat; // matriz de adjascencia

    int tstmp; // timestamp para o dsf

    char label;
    // char pad[3]; // padding para alinhar a struct na memoria
};

struct block
// subgrafo de vertices fortemente conexos
{
    struct vertex **vertexes;
    size_t n;
};

// macro para calcular o menor de dois valores
#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

struct graph *fromAgraph_t(Agraph_t *g);
void printGraph(struct graph *g);
void dfs(struct graph *g);
void dfsAux(struct graph *g, struct vertex *v);
void decomposeDif(struct graph *g);
void decomposeAux(struct graph *g, struct vertex *r, int c);
void lowPoint(struct graph *g);
void lowPointAux(struct graph *g, struct vertex *r);
void findArticulations(struct graph *g);

struct block *separateBlocks(struct graph *g);
void printBlocks(struct block *bls, size_t n);

int compareLabel(const void *a, const void *b);
int comparePostOrderR(const void *a, const void *b);

struct graph *fromAgraph_t(Agraph_t *g)
// converte do cgraph para a struct graph
{
    struct graph *new = malloc(sizeof(struct graph));
    if (!new)
    {
        perror("Falha ao alocar memória");
        exit(EXIT_FAILURE);
    }

    new->label = agnameof(g)[0];
    new->n = (size_t)agnnodes(g); // vertices
    new->m = (size_t)agnedges(g); // arestas

    // cria a lista de vertices
    new->vertexes = malloc(sizeof(struct vertex *) * (size_t) new->n);
    if (!new->vertexes)
    {
        perror("Falha ao alocar memória");
        exit(EXIT_FAILURE);
    }
    int i = 0;
    for (Agnode_t *v = agfstnode(g); v; v = agnxtnode(g, v))
    {
        new->vertexes[i] = malloc(sizeof(struct vertex));
        if (!new->vertexes[i])
        {
            perror("Falha ao alocar memória");
            exit(EXIT_FAILURE);
        }
        new->vertexes[i]->label = agnameof(v)[0];
        new->vertexes[i]->is_articulation = 0;
        ++i;
    }
    qsort(new->vertexes, new->n, sizeof(struct vertex *), compareLabel);

    // cria a matrix de adjascencia
    new->adj_mat = calloc(sizeof(short), (size_t) new->n *(size_t) new->n);
    for (size_t j = 0; j < new->n; j++)
    {
        new->vertexes[j]->index = (int)j;
        Agnode_t *v = agnode(g, (char[2]){(char)new->vertexes[j]->label, '\0'}, FALSE);
        for (Agedge_t *e = agfstout(g, v); e; e = agnxtout(g, e))
        {
            Agnode_t *u = aghead(e) != v ? aghead(e) : agtail(e);

            int r = (agnameof(v)[0] - 97);
            int c = (agnameof(u)[0] - 97);
            new->adj_mat[r * (int)new->n + c] = 1;

            if (agisundirected(g))
            {
                new->adj_mat[c * (int)new->n + r] = 1;
            }
        }
    }

    return new;
}

void dfs(struct graph *g)
// busca em profundidade
{
    for (size_t i = 0; i < g->n; i++)
    {
        g->vertexes[i]->state = 0;
    }
    g->tstmp = 0;

    for (size_t i = 0; i < g->n; i++)
    {
        if (g->vertexes[i]->state == 0)
        {
            g->vertexes[i]->father = NULL;
            dfsAux(g, g->vertexes[i]);
        }
    }
}

void dfsAux(struct graph *g, struct vertex *r)
// auxiliar da busca em profundidade
{
    r->pre = ++(g->tstmp);
    r->state = 1;
    for (size_t i = 0; i < g->n; i++)
    {
        if ((g->adj_mat[(size_t)r->index * g->n + i] == 1) && (g->vertexes[i]->state == 0))
        {
            g->vertexes[i]->father = r;
            dfsAux(g, g->vertexes[i]);
        }
    }
    r->state = 2;
    r->post = ++(g->tstmp);
}

int compareLabel(const void *a, const void *b)
// compara os labels do vertice em ordem lexicografica
{
    return (((const struct vertex *)(*((const struct vertex *const *)a)))->label - ((const struct vertex *)(*((const struct vertex *const *)b)))->label);
}

int comparePostOrderR(const void *a, const void *b)
// compara os vertices pela pos ordem
{
    return (((const struct vertex *)(*((const struct vertex *const *)b)))->post - ((const struct vertex *)(*((const struct vertex *const *)a)))->post);
}

void decomposeAux(struct graph *g, struct vertex *r, int c)
{
    r->state = 1;
    for (size_t i = 0; i < g->n; i++)
    {
        struct vertex *v = g->vertexes[i];
        if ((g->adj_mat[(size_t)r->index * g->n + i] == 1) && (v->state == 0) && (!v->is_articulation))
        {
            decomposeAux(g, g->vertexes[i], c);
        }
    }
    r->component = c;
    r->state = 2;
}

void decomposeDif(struct graph *g)
// faz a decomposicao do grafo em componentes ignorando os vertices de corte
{
    // reverso da pos order da busca em profundidade
    struct vertex **l = malloc(sizeof(struct vertex *) * g->n);
    memcpy(l, g->vertexes, g->n * sizeof(struct vertex *));
    qsort(l, g->n, sizeof(struct vertex *), comparePostOrderR);

    for (size_t i = 0; i < g->n; i++)
    {
        g->vertexes[i]->state = 0;
        g->vertexes[i]->component = 0;
    }
    int c = 0;

    for (size_t i = 0; i < g->n; i++)
    {
        if ((l[i]->state == 0) && (!l[i]->is_articulation))
        {
            decomposeAux(g, l[i], ++c);
        }
    }

    g->comp_num = (size_t)c;
}

void lowPoint(struct graph *g)
// calcula o low point para cada vertice
{
    // inicializa
    for (size_t i = 0; i < g->n; i++)
    {
        g->vertexes[i]->state = 0;
        g->vertexes[i]->father = NULL;
        g->vertexes[i]->son_num = 0;
    }

    // percorre os vertices
    for (size_t i = 0; i < g->n; i++)
    {
        if (g->vertexes[i]->state == 0)
        {
            g->vertexes[i]->low = g->vertexes[i]->lv = 0;
            lowPointAux(g, g->vertexes[i]);
        }
    }
}

void lowPointAux(struct graph *g, struct vertex *r)
// funcao auxiliar para o low point
{
    r->state = 1;

    // percorre a vizinhanca de r
    for (size_t i = 0; i < g->n; i++)
    {
        if (g->adj_mat[(size_t)r->index * g->n + i] == 1)
        {

            struct vertex *w = g->vertexes[i];
            if ((w->state == 1) && (w != r->father))
            {
                r->low = min(r->low, w->lv);
            }
            else if (w->state == 0)
            {
                r->son_num++;
                w->father = r;
                w->low = w->lv = r->lv + 1;
                lowPointAux(g, w);
                r->low = min(r->low, w->low);
            }
        }
    }
    r->state = 2;
}

void findArticulations(struct graph *g)
// encontra os vertices de corte do grafo
{
    lowPoint(g);

    for (size_t i = 0; i < g->n; i++)
    {
        struct vertex *u = g->vertexes[i];
        if (u->father == NULL) // u é raiz
        {
            if (u->son_num >= 2)
            {
                u->is_articulation = 1;
            }
        }
        else
        {
            // percorre os filhos
            for (size_t j = 0; j < g->n; j++)
            {
                struct vertex *v = g->vertexes[j];
                if ((g->adj_mat[i * g->n + j] == 1) && (u == v->father))
                {
                    if (u->lv <= v->low)
                    {
                        u->is_articulation = 1;
                        break;
                    }
                }
            }
        }
    }
}

struct block *separateBlocks(struct graph *g)
// separa os blocos do grafo
{
    struct block *bls = malloc(sizeof(struct block) * g->comp_num);
    for (size_t i = 0; i < g->comp_num; i++)
    {
        bls[i].vertexes = malloc(sizeof(struct vertex *) * g->n);
        bls[i].n = 0;
    }

    for (size_t i = 0; i < g->n; i++)
    {
        struct vertex *u = g->vertexes[i];
        // vertices de corte fazem parte de mais de uma componente
        if (u->is_articulation)
        {
            // percorre os filhos
            for (size_t j = 0; j < g->n; j++)
            {
                struct vertex *v = g->vertexes[j];
                if ((g->adj_mat[i * g->n + j] == 1) && !v->is_articulation)
                {
                    size_t comp_size = bls[v->component - 1].n;
                    if (bls[v->component - 1].vertexes[comp_size - 1] != u)
                    {
                        bls[v->component - 1].vertexes[comp_size] = u;
                        bls[v->component - 1].n++;
                    }
                }
            }
        }
        else
        {
            bls[u->component - 1].vertexes[bls[u->component - 1].n] = u;
            bls[u->component - 1].n++;
        }
    }

    return bls;
}

void printBlocks(struct block *bls, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        qsort(bls[i].vertexes, bls[i].n, sizeof(struct vertex *), compareLabel);
        for (size_t j = 0; j < bls[i].n; j++)
        {
            printf("%c ", bls[i].vertexes[j]->label);
        }
        printf("\n");
    }
}

void printGraph(struct graph *g)
// imprime os vertices e a matriz de adjascencia
{
    printf("V = {");
    for (size_t i = 0; i < g->n; i++)
    {
        struct vertex *v = g->vertexes[i];
        printf("%c comp=%d art=%d, ", v->label, v->component, v->is_articulation);
    }
    printf("\n\n ");

    for (size_t i = 0; i < g->n; i++)
    {
        printf(" %c", g->vertexes[i]->label);
    }

    for (size_t i = 0; i < g->n; i++)
    {
        printf("\n%c", g->vertexes[i]->label);
        for (size_t j = 0; j < g->n; j++)
        {
            printf(" %d", g->adj_mat[i * g->n + j]);
        }
    }
    printf("\n");
}

int main(void)
{
    Agraph_t *ag = agread(stdin, NULL);
    // agwrite(ag, stdout);

    struct graph *g = fromAgraph_t(ag);
    findArticulations(g);
    dfs(g);
    decomposeDif(g);
    struct block *bls = separateBlocks(g);
    printBlocks(bls, g->comp_num);

    return EXIT_SUCCESS;
}