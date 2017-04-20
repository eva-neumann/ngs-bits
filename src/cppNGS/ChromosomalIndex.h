#ifndef CHROMOSOMALINDEX_H
#define CHROMOSOMALINDEX_H
#include "cppNGS_global.h"
#include "limits"
#include "Chromosome.h"
#include "IntervalTree.h"
#include <unordered_map>
#include <utility>
#include <map>
#include <QVector>
//#include <QTextStream>
//#include "Helper.h"


template <typename T>
using OrderedTreeMap = std::map<int, IntervalTree<T> >;
template <typename T>
using OrderedTreeMapPair= std::pair<int, IntervalTree<T> >;

using UnorderedIndexMap = std::unordered_map<int, std::list<int> >;
using UnorderedIndexMapPair= std::pair<int, std::list<int> >;

///Chromosomal index for fast access to @em sorted containers with chromosomal range elements like BedFile and VariantList.
template <class T>
class CPPNGSSHARED_EXPORT ChromosomalIndex
{
public:
    ///Constructor.
    ChromosomalIndex(const T& container, int depth=std::numeric_limits<int>::max(), int maxbucket=1);

    ///Re-creates the index (only needed if the container content changed after calling the index constructor).
    void createIndex();

    ///Returns the underlying container
    const T& container() const { return container_; }

    /// Returns the interval tree of a certain chromosome
    const IntervalTree<T>* intervalTree(const Chromosome& chrom) const;

    ///Returns a vector of element indices overlapping the given chromosomal range.
    QVector<int> matchingIndices(const Chromosome& chr, int start, int end) const;
    ///Returns the index of the first element in the container that overlaps the given chromosomal range, or -1 if no element overlaps.
    int matchingIndex(const Chromosome& chr, int start, int end) const;

protected:
    const T& container_;
    OrderedTreeMap<T> chr_intervals_;
    int depth_;
    int maxbucket_;
};

template <class T>
ChromosomalIndex<T>::ChromosomalIndex(const T& container, int depth, int maxbucket)
    : container_(container),
      depth_(depth),
      maxbucket_(maxbucket)
{
    createIndex();
}

template <class T>
void ChromosomalIndex<T>::createIndex()
{
    UnorderedIndexMap hash_indices;
    UnorderedIndexMap::iterator hash_it;
    for (int i=0; i < container_.count(); ++i)
    {
        const Chromosome& chr = container_[i].chr();
        hash_it=hash_indices.find(chr.num());
        if (hash_it==hash_indices.end())
        {
            std::list<int> v({i});
            UnorderedIndexMapPair initial_list(chr.num(),v);
            hash_indices.insert(initial_list);
        }
        else
        {
            hash_it->second.push_back(i);
        }
    }
    // build for each chromosome an interval tree
    hash_it = hash_indices.begin();
    while (hash_it != hash_indices.end())
    {
        IntervalTree<T> interval_tree(container_, hash_it->second, 0, 0);
        OrderedTreeMapPair<T> key_tree_pair(hash_it->first,interval_tree);
        chr_intervals_.insert(key_tree_pair);
        ++hash_it;
    }
}

template <class T>
QVector<int> ChromosomalIndex<T>::matchingIndices(const Chromosome& chr, int start, int end) const
{
    QVector<int> matching_indices;
    typename std::map<int, IntervalTree<T> >::const_iterator tree_it = chr_intervals_.find(chr.num());

    //chromosome not found
    if (tree_it == chr_intervals_.end()) return matching_indices;

    tree_it->second.overlappingIntervals(start,end,matching_indices);

    return matching_indices;
}


template <class T>
int ChromosomalIndex<T>::matchingIndex(const Chromosome& chr, int start, int end) const
{
    //chromosome not found
    typename std::map<int, IntervalTree<T> >::const_iterator tree_it = chr_intervals_.find(chr.num());
    if (tree_it == chr_intervals_.end()) return -1;

    QVector<int> matching_indices;
    tree_it->second.overlappingIntervals(start,end,matching_indices,true);
    if (matching_indices.count() > 0)
    {
        return matching_indices[0];
    }
    return -1;
}

#endif // CHROMOSOMALINDEX_H
