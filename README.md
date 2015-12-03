Welcome to LM-SDSL

# compile instructions

1. Check out reprository
2. `git submodule init`
3. `git submodule update`
4. `cd build`
5. `cmake ..`

# usage

Create a collection:

```
./create-collection.x -i toyfile.txt -c ../collections/toy
```

Build index

```
./build-index.x -c ../collections/toy/
```

Query index

```
./query-index-stupid.x -c ../collections/toy/ -p toyquery.txt
```

## Running `unit' tests ##

To run the unit-test.x binary first you need to do the following
```
rm -r ../collections/unittest/
./create-collection.x -i ../UnitTestData/data/training.data -c ../collections/unittest
./unit-test.x
```
## Comparison on Europarl German KenLM v.s. Accurate v.s Fishy ##
Training raw size: 170MB
KENLM - on eu_de and 500 test sentences (includes OOVs:	80)
```
2-gram:	166.68	
default: 95MB    trie: 38MB
3-gram:	108.44
default: 402MB    trie: 168MB
4-gram:	100.75
default: 963MB    trie: 441MB
5-gram:	99.59
default: 1692MB    trie: 825MB
```
under the same training and test setting the accurate v.s. fishy:
```
2-gram:	166.68  v.s.  166.68
275MB
3-gram:	108.44  v.s.  108.09
275MB
4-gram:	100.75  v.s.  99.68
275MB
5-gram:	99.59   v.s.  97.94
275MB
```
on larger test set with 10K sentences the fishy v.s. kenlm
```
2-gram 174.80  v.s.  174.80
3-gram 112.47  v.s.  112.84
4-gram 104.78  v.s.  105.86
5-gram 103.11  v.s.  104.82
```
