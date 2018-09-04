# RadixTree

## What

A radix tree in c++ 14. Smart Pointer, Mutex, stl containers.

## How

Feed the radixTree with the `loadString` method.

Alternaly, you can use the CLI. Run the tree with the `-d` flag and then :
- feed the tree via `STDIN` (or, for example, redirect a file with `<`)
- enter the key word `start` to start searching in the tree

Output example :
```
~/repo/sbc/src/radix/build master_1234_radix
11:07:09 ❯ ./radix_tree -d -v
Enter some key to add (or start to start to search) : dog
Enter some key to add (or start to start to search) : did
Enter some key to add (or start to start to search) : dogs
Enter some key to add (or start to start to search) : doe
Enter some key to add (or start to start to search) : start
There is 4 entries for 6 leafs.

----------[LeafID]:[1]----------
[g]/[dog] = [0]
[RootID]:	5
[ChildIDs]:	4
----------[LeafID]:[2]----------
[d]/[] = []
[RootID]:	0
[ChildIDs]:	3 5
----------[LeafID]:[3]----------
[id]/[did] = [1]
[RootID]:	2
[ChildIDs]:
----------[LeafID]:[4]----------
[s]/[dogs] = [2]
[RootID]:	1
[ChildIDs]:
----------[LeafID]:[5]----------
[o]/[] = []
[RootID]:	2
[ChildIDs]:	6 1
----------[LeafID]:[6]----------
[e]/[doe] = [3]
[RootID]:	5
[ChildIDs]:
```

### Usage

```
~/repo/sbc/src/radix/build master_1234_radix
18:26:00 ❯ ./radix_tree -h -d
Usage : ./radix_tree [-v|-vv|-vvv] -h

        -v|-vv|-vvv:    Level of verbosity of the radix trie.
        -d:             Enable the cli / debug mode.
        -h:             Display this screen.

```

### Build

#### Requirement
 - cmake & make
 - clang or gcc (c++14 compatible)

#### Created
 - a binary file `radix_tree`; which is the RadixTree class compiled with a funny CLI
 - a shared library, `libradix.so`; ready to be integrated :nerd_face:

#### Use it

The `RadixTree` class take a `void *` as value indexed. To be quick, the radix tree index with `std::string` key and store some `void *` data.

#### Build it

```
$ cd build
$ cmake ../
$ # OR to build with debug flag
$ cmake ../ -DDEFINE_DEBUG_RADIX=1
$ # OR to build with debug compile flag
$ cmake ../ -DDEFINE_DEBUG_COMPILE=1
$ make -j4
```

## TODO

- [x] longest prefix match method :rocket:
- [x] cli match
- [x] cli feed
- [x] templated value
- [x] threadSafe insertion
- [x] perf test
- [x] size test
- [x] timer research
- [x] doc
- [ ] patricia rb :cry: