<div align="center">
<h1>GQLite</h1>
</div> 

[![Join the chat at https://gitter.im/webbery/gqlite](https://badges.gitter.im/webbery/gqlite.svg)](https://gitter.im/webbery/gqlite?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
<a href="https://github.com/webbery/gqlite/actions">
<img src="https://img.shields.io/github/workflow/status/webbery/gqlite/CMake/master" alt="Github">
</a>
<a href="https://codecov.io/gh/webbery/gqlite">
  <img src="https://codecov.io/gh/webbery/gqlite/branch/master/graph/badge.svg?token=3S0K5GCIYC"/>
</a>

This is the expriments for testing abilities of graph database in ending device.  

<!-- vscode-markdown-toc -->
* 1. [Designed](#Designed)
* 2. [The Spec of Implements](#TheSpecofImplements)
* 3. [Build](#Build)
	* 3.1. [ubuntu](#ubuntu)
	* 3.2. [macos](#macos)
	* 3.3. [windows](#windows)
* 4. [Graph Query Language](#GraphQueryLanguage)
	* 4.1. [Create Graph](#CreateGraph)
	* 4.2. [Data Types](#DataTypes)
	* 4.3. [Add Vertex & Edge](#AddVertexEdge)
	* 4.4. [Remove Vertex & Edge](#RemoveVertexEdge)
	* 4.5. [Query](#Query)
		* 4.5.1. [intrinct function](#intrinctfunction)
		* 4.5.2. [condition](#condition)
	* 4.6. [Inference](#Inference)
	* 4.7. [Ceate Job](#CeateJob)
	* 4.8. [Transition](#Transition)
* 5. [Utility](#Utility)
	* 5.1. [Show Graphs](#ShowGraphs)
	* 5.2. [Extension](#Extension)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->
##  1. <a name='Designed'></a>Designed
In order to get more inflexibility, we try to impliment graph database on this paper: GRAD: On Graph Database Modeling. But not strict.
### Operator
#### Graph Matching <img src="https://render.githubusercontent.com/render/math?math=P">
This operator is not extactly a graph matching. It can be a fuzzy matching.
#### Walking <img src="https://render.githubusercontent.com/render/math?math=W">
Walking is used to be through graph. So it's best to select a well walking method. Such as `Random Walk`, `A*` and so on.
##  2. <a name='TheSpecofImplements'></a>The Spec of Implements
| Feature | Description | Implements | Version |
| :--------:  | :--------: | :--------: | :--------: |
| create graph | | &check; | 0.0.1 |
| upset vertex | | &check; | 0.0.1 |
| query vertex | | &check; | 0.0.1 |
| binary support | support binary data | &check; | 0.0.1 |
| drop graph | | &check; | 0.0.1 |
| delete vertex | | &check; | 0.0.1 |
| upset edge | | - | 0.1.0 |
| query edge | | -| 0.1.0 |
| delete edge | | - | 0.1.0 |
| conectivity | | - | 0.1.0 |
| N-adjacent | | - | 0.1.0 |
| walk of BFS | | - | 0.1.0 |
| walk of DFS | | - | 0.1.0 |
| walk of dijk | | - | 0.1.0 |
| js support | implement a subset/extend of javasript for complex query. |  | 0.2.0 |
| walk of A* | A* algrithm for graph search |  | 0.2.0 |
| GiST index | boosting for more type search |  | 0.3.0 |
| extension support | interface for GiST |  | 0.3.0 |
| HMM | bayes network support |  | 0.4.0 |
| CRF | bayes network support |  | 0.4.0 |

##  3. <a name='Build'></a>Build
Use command with `git clone --recursive https://github.com/webbery/gqlite.git` to clone this repository.
###  3.1. <a name='ubuntu'></a>ubuntu  
Install latest version of bison.
###  3.2. <a name='macos'></a>macos  
###  3.3. <a name='windows'></a>windows  
An version of flex&bison is placed in dir `tool`. So it's not need to install dependency.

##  4. <a name='GraphQueryLanguage'></a>Graph Query Language
###  4.1. <a name='CreateGraph'></a>Create Graph
Create a graph is simply use `create` keyword. The keyword of `group`, means that all entity node which group belongs to. If we want to search vertex by some property, `index` keyword will regist it.
```javascript
{
    create: 'movielens',
    group: [
        {movie: ['title', 'genres']},
        {tag: ['user_id', 'tag', 'movie_id']},  // <-- relationship's property must write center if it is a edge
        {rate: ['user_id', 'rate', 'movie_id']}
    ],
    index: 'tag',
}
```
Here we create an index called `tag`. The `tag` will create revert index from `tag` to group `tag`'s id. Following create a index pair with key and value:
```javascript
{
    create: 'movielens',
    group: [
        {movie: ['title', 'genres']},
        {tag: ['user_id', 'movie_id', 'tag']},
        {rate: ['user_id', 'movie_id', 'rate']}
    ],
    index: {tag: 'movie_id'},       // <-- index pair
}
```
So after upset a new tag, the revert index will be added.
###  4.2. <a name='DataTypes'></a>Data Types
Normaly, basic data type as follows:  
    **string**: 'string'  
    **number**: 10 means integer, 10.0 means real number.   
    **array**: start as `[` and end with `]`  
    **binary**:  start with `b`, then follow as base64 string, it will save as binary data. Such as `b'df32099'`  
    **datetime**: start with `d`, then will try to convert following string to datetime, such as `d'1642262159'`  
    **vector**: a special type of array, which items are same type.  
    **hash**: a special type of string, start with `h` like `h'hash'`  
###  4.3. <a name='AddVertexEdge'></a>Add Vertex & Edge
add or update vertex:
```javascript
{
    upset: 'movie',
    vertex:[
        [21, {'title': 'Get Shorty', genres: ['Comedy', 'Crime', 'Thriller']}],
        [53, {title: 'Lamerica (1994)', genres: ['Adventure','Drama']}],
        [88, {title: 'Black Sheep (1996)'}]
    ]
}
```
Note that current graph is created graph before called `movielens`. The 3 of vertexes is added to group `movie`.    

add or update edge:
```javascript
{
    upset: 'tag',
    edge: [
        [{user_id: 2}, {'--': 'Martin Scorsese'}, {movie_id: [106782, 89774]}],
        [{user_id: 21}, {'--': ['romantic comedy', 'wedding']}, {movie_id: 1569}],
    ]
}
```
For simply use, it can be write as follows, but id is automatic generated by database:
```javascript
{
    upset: 'edge',
    edge: [
        ['Tom', ->, 'Lamerica'],
        ['Kitty', <-, 'Black Sheep'],
    ]
}
```
or simply use bidirection:
```javascript
{
    upset: 'tag',
    edge: [
        ['Tom', --, 'Lamerica'],
        ['Kitty', --, 'Black Sheep'],
    ]
}
```
###  4.4. <a name='RemoveVertexEdge'></a>Remove Vertex & Edge
```javascript
{remove: 'graph', vertex: [21, 88]}
```
###  4.5. <a name='Query'></a>Query
There are three kinds of query. First is `vertex` which is used to find vertexes instance.  Another one is `edge` which is used to find edges instance. The last one is `path` which is used to find a batch of path from some points to other points.
####  4.5.1. <a name='intrinctfunction'></a>intrinct function
##### count()
```javascript
{// this is used to count the number of vertex
    query: count(vertex),
    group: 'movie'
}
```
####  4.5.2. <a name='condition'></a>condition
query all movie that has tag:
```javascript
{
    query: [movie.title, movie.genres],
    where: [
        [user_id, {--: *}, movie_id]        // here is an edge condition, user_id and movie_id are in group `tag`, * represent all relationship here.
    ],
    in: 'movielens' // the graph instance can be written here or not.
}
```
<!-- ```javascript
{
    query: [movie, tag],
    where: [
        [user_id, {--: 'wedding'}, movie_id]
    ],
    in: 'graph'
}
``` -->
query a reverted-index:
```javascript
{
    query: movie,
    where: {tag: ['black comedy']}
}
```
query points from graph by relationship:
```javascript
{
    query: user,
    where: {
        ->: 'son'
    }
}
```
```javascript
{
    query: user,
    where: [
        {
            user: function(user) { return user.age > 10}
        }
    ],
}
```
query a list of neighbors, where `1` mean 1'st neighbors:
```javascript
{query: user, from: 'v1', where: {--: 1}}
```
In order to get a search way, you can use `path` to archive it.
```javascript
{query: path, from: 'v1', to: 'v2', where: {--: 'dijk'}}
```
which `dijk` means dijk search.
###  4.6. <a name='Inference'></a>Inference
Here we define a kind of inference operator, and apply it to a graph.  
First Order Logic:
```javascript
{
}
```
HMM:
```javascript
{
    query: hidden_variant,
    event: [{e1: 'sun'}, {e2: 'rain'}, {e3: 'wind'}],
    where: [
        [hidden_variant.v1, {->: 0.2}, e1],
        [hidden_variant.v2, {->: gassian(0.2, 0.1)}, e2],
        [hidden_variant.v3, {->: gassian(0.2, 0.1)}, e3],
        [hidden_variant.v1, {->: 0.2}, hidden_variant.v2],
        [hidden_variant.v2, {->: 0.2}, hidden_variant.v3],
        [hidden_variant.v1, {->: 0.2}, hidden_variant.v4],
    ]
}
```
###  4.7. <a name='CeateJob'></a>Ceate Job
###  4.8. <a name='Transition'></a>Transition
##  5. <a name='Utility'></a>Utility
###  5.1. <a name='ShowGraphs'></a>Show Graphs
```
show graph
show graph 'xxx'
```
###  5.2. <a name='Extension'></a>Extension
```
install extension geo_bound
```
### Use Graph
```
use graph 'xxx'
```

## Papers  
1. Yihan Sun, Daniel Ferizovic, Guy E. Belloch. PAM: Parallel Augmented Maps.  
2. Laxman Dhulipala, Guy Blelloch, Yan Gu, Yihan Sun. PaC-trees: Supporting Parallel and Compressed Purely-Functional Collections.  
3. Amine Ghrab, Oscar Romero, Sabri Skhiri etc. GRAD: On Graph Database Modeling.  
4. [向量索引算法HNSW和NSG的比较](https://zhuanlan.zhihu.com/p/105594786)  