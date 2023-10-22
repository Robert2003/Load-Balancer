**Name: Damian Mihai-Robert**<br>

<h1 style="text-align: center;">Load balancer</h1>

<br>

# Description:

* The program simulates a Load Balancer, using Consistent Hashing for storing objects on servers.
  * The servers are stored as an array, each having a memory that is represented by a hashtable.
  * Each hashtable uses direct chaining for solving the hash collisions.
  * If a server is being added on the ring, the next server could be having objects whose hashes are smaller than the new server's id hash, so they need to be rebalanced.
  * If a server is being removed from the ring, all his objects should be redistributed.
* Additional explanations on the code:
  * When rebalancing after a server addition, only the objects with smaller hashes should be redistributed, but that didn't work for me, so I redistributed every object in that server, even if it gets added on the same server.

<br>

# Comments:

* Q: Do you think you could have done a better implementation?
  * A: Yes, especially at rebalancing after a new server is added.
* Q: What did you learn from doing this assignment?
  * A: I learned about how to efficiently store servers and objects on them.
* Other comments:
  * A: You could consider adding a clear instruction at the beginning of the problem statement, highlighted in bold, to indicate that the hashring should be stored as an array, as it may be easier to implement the Load Balancer using double linked lists.

<br>

# Possible commands:

<h2><b>Load balancer:</b></h2>

1. `store`

### Usage:

```
loader_store(main_server, key, value, &index_server);
```
<br>

2. `retrieve`

### Usage:

```
loader_retrieve(main_server, key, &index_server);
```
<br>

3. `add_server`

### Usage:

```
loader_add_server(main_server, server_id);
```
<br>

4. `remove_server`

### Usage:

```
loader_remove_server(main_server, server_id);
```
<br>
<br>

<h2><b>Server:</b></h2>

1. `server_store`

### Usage:

```
server_store(server_memory, key, value);
```
<br>

2. `server_retrieve`

### Usage:

```
server_retrieve(server_memory, key);
```
<br>

3. `server_remove`

### Usage:

```
server_remove(server_memory, key);
```
<br>

4. `free_server_memory`

### Usage:

```
free_server_memory(server_memory);
```
<br>
