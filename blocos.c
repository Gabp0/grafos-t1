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
    short *adj_mat; // matriz de adjascencia

    int tstmp; // timestamp para o dsf

    char label;
    char pad[3]; // padding para alinhar a struct na memoria
};

int tstmp;

struct graph *fromAgraph_t(Agraph_t *g);
void printGraph(struct graph *g);
void dfs(struct graph *g);
void dfsAux(struct graph *g, struct vertex *v);
void decompose(struct graph *g);
void decomposeAux(struct graph *g, struct vertex *r, int c);

void copyVertex(struct vertex *u, struct vertex *v);
int compareLabel(const void *a, const void *b);
int comparePostOrderR(const void *a, const void *b);

void copyVertex(struct vertex *u, struct vertex *v)
// copia em u o vertice v
{
    u->component = v->component;
    u->index = v->index;
    u->label = v->label;
    u->post = v->post;
    u->pre = v->pre;
    u->state = v->state;
}

int compareLabel(const void *a, const void *b)
{
    return (((const struct vertex *)(*((const struct vertex *const *)a)))->label - ((const struct vertex *)(*((const struct vertex *const *)b)))->label);
}

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
            new->adj_mat[c * (int)new->n + r] = 1;
        }
    }

    return new;
}

void dfsAux(struct graph *g, struct vertex *r)
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

void dfs(struct graph *g)
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

int comparePostOrderR(const void *a, const void *b)
{
    return (((const struct vertex *)(*((const struct vertex *const *)b)))->post - ((const struct vertex *)(*((const struct vertex *const *)a)))->post);
}

void decomposeAux(struct graph *g, struct vertex *r, int c)
{
    r->state = 1;
    for (size_t i = 0; i < g->n; i++)
    {
        if ((g->adj_mat[(size_t)r->index * g->n + i] == 1) && (g->vertexes[i]->state == 0))
        {
            decomposeAux(g, g->vertexes[i], c);
        }
    }
    r->component = c;
    r->state = 2;
}

void decompose(struct graph *g)
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
        if (l[i]->state == 0)
        {
            decomposeAux(g, l[i], ++c);
        }
    }
}

void printGraph(struct graph *g)
// imprime os vertices e a matriz de adjascencia
{
    printf("V = {");
    for (size_t i = 0; i < g->n; i++)
    {
        printf("%c pos=%d comp=%d, ", g->vertexes[i]->label, g->vertexes[i]->post, g->vertexes[i]->component);
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
    dfs(g);
    decompose(g);
    printGraph(g);

    return EXIT_SUCCESS;
}