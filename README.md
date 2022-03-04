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
###  3.1. <a name='ubuntu'></a>ubuntu  
Install latest version of bison.
###  3.2. <a name='macos'></a>macos  
###  3.3. <a name='windows'></a>windows  
An version of flex&bison is placed in dir `tool`. So it's not need to install dependency.

##  4. <a name='GraphQueryLanguage'></a>Graph Query Language
###  4.1. <a name='CreateGraph'></a>Create Graph
Create a graph is simply use `create` keyword`. If we want to search vertex by some property, `index` keyword will regist it.
```javascript
{
    create: 'graph',
    index: 'keyword',
}
```
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
    upset: 'graph',
    vertex:[
        ['Tom', {'age': 10}],
        ['Kitty', {'sex': 'female'}],
        ['Jimmy', {'sex': 'male', 'age': 35}]
    ]
}
```
Note that `Tom`, `Kitty` and `Jimmy` is id of vertex.  

add or update edge:
```javascript
{
    upset: 'graph',
    edge: [
        ['Tom', {'->': 'son'}, 'Jimmy'],
        ['Kitty', {'<-': 'wife'}, 'Jimmy'],
    ]
}
```
if there is no edge property, it can be written simply as follows:
```javascript
{
    upset: 'graph',
    edge: [
        ['Tom', ->, 'Jimmy'],
        ['Kitty', <-, 'Jimmy'],
    ]
}
```
or simply use bidirection:
```javascript
{
    upset: 'graph',
    edge: [
        ['Tom', --, 'Jimmy'],
        ['Kitty', --, 'Jimmy'],
    ]
}
```
###  4.4. <a name='RemoveVertexEdge'></a>Remove Vertex & Edge
```javascript
{remove: 'graph', vertex: ['Tom']}
```
###  4.5. <a name='Query'></a>Query
There are three kinds of query. First is `vertex` which is used to find vertexes instance.  Another one is `edge` which is used to find edges instance. The last one is `path` which is used to find a batch of path from some points to other points.
####  4.5.1. <a name='intrinctfunction'></a>intrinct function
##### count()
```javascript
{// this is used to count the number of vertex
    query: count(vertex),
    in: 'graph'
}
```
####  4.5.2. <a name='condition'></a>condition
```javascript
{
    query: [vertex.sex, vertex.age],
    where: [
        {<->: 'son'}
    ],
    in: 'graph'
}
```
```javascript
{
    query: [vertex.sex, vertex.age],
    where: [
        {age: {$gt: 20}}
    ],
    in: 'graph'
}
```
query a reverted-index:
```
```
query points from graph by relationship:
```javascript
{
    query: vertex,
    from: 'p',
    where: {
        ->: 'son'
    },
    in: 'graph'
}
```
```javascript
{
    query: vertex,
    from: 'p',
    where: [
        {
            ->: 'friend',
            vertex: function(vertex) { return vertex.age > 10}
        }
    ],
    in: 'graph'
}
```
query a list of neighbors, where `1` mean 1'st neighbors:
```javascript
{query: vertex, from: 'v1', where: {id: 'v1', --: 1}, in: 'graph'}
```
In order to get a search way, you can use `path` to archive it.
```javascript
{query: path, in: 'graph', from: 'v1', to: 'v2', where: {--: 'dijk'}}
```
which `dijk` means dijk search.
###  4.6. <a name='Inference'></a>Inference
Here we define a kind of inference operator, and apply it to a graph.  
First Order Logic:
```javascript
{
    infer: '*',
    start: 'p',
    in: 'graph',
    define: function reverse(relation) { return 'father'}
}
```
HMM:
```javascript
{
    infer: '*',
    start: ['p1', 'p2', 'p3'],
    in: 'graph',
    '->': function(from, to) {return 0.5;}
}
```
###  4.7. <a name='CeateJob'></a>Ceate Job
###  4.8. <a name='Transition'></a>Transition
##  5. <a name='Utility'></a>Utility
###  5.1. <a name='ShowGraphs'></a>Show Graphs
```
show graph
```
###  5.2. <a name='Extension'></a>Extension
```
install extension geo_bound
```