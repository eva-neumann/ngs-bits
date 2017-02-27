#ifndef CHROMOSOMALINDEX_H
#define CHROMOSOMALINDEX_H
#include "cppNGS_global.h"
#include "limits"
#include "Chromosome.h"
#include "IntervalTree.h"
#include <QHash>
#include <QVector>
#include <QPair>

///Chromosomal index for fast access to @em sorted containers with chromosomal range elements like BedFile and VariantList.
template <class T>
class CPPNGSSHARED_EXPORT ChromosomalIndex
{
public:
    ///Constructor.
    ChromosomalIndex(const T& container, int depth=3000, int maxbucket=128);

    ///Re-creates the index (only needed if the container content changed after calling the index constructor).
    void createIndex();

    ///Returns the underlying container
    const T& container() const { return container_; }

    ///Returns a vector of element indices overlapping the given chromosomal range.
    QVector<int> matchingIndices(const Chromosome& chr, int start, int end) const;
    ///Returns the index of the first element in the container that overlaps the given chromosomal range, or -1 if no element overlaps.
    int matchingIndex(const Chromosome& chr, int start, int end) const;

protected:
    const T& container_;
    QHash<int, IntervalTree > chr_intervals_;
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
    // collect the intervals for each chromosome
    QHash<int, QVector<Interval> > hash_intervals;
    for (int i=0; i<container_.count(); ++i)
    {
        Chromosome chr = container_[i].chr();
        if (!hash_intervals.contains(chr.num()))
        {
            QVector<Interval> intervals;
            hash_intervals.insert(chr.num(),intervals);
        }
        Interval interval(container_[i].start(),container_[i].end(),i);
        hash_intervals[chr.num()].append(interval);
    }

    // build for each chromosome an interval tree
    QHash<int, QVector<Interval> >::const_iterator hash_intervals_it = hash_intervals.constBegin();
    while (hash_intervals_it != hash_intervals.constEnd())
    {
        IntervalTree interval_tree(hash_intervals_it.value(),depth_,0,0,maxbucket_);
        chr_intervals_.insert(hash_intervals_it.key(),interval_tree);
        ++hash_intervals_it;
    }

}

template <class T>
QVector<int> ChromosomalIndex<T>::matchingIndices(const Chromosome& chr, int start, int end) const
{
    QVector<int> matching_indices;

    //chromosome not found
    if (!chr_intervals_.contains(chr.num())) return matching_indices;

    /// Eva: hier wird beim operator[] IntervalTree CpCstr aufgerufen :-|
    QVector<Interval> matching_intervals = chr_intervals_[chr.num()].overlappingIntervals(start,end);

    foreach (const Interval& interval, matching_intervals)
    {
        matching_indices.append(interval.value);
    }

    return matching_indices;
}


template <class T>
int ChromosomalIndex<T>::matchingIndex(const Chromosome& chr, int start, int end) const
{
    //chromosome not found
    if (!chr_intervals_.contains(chr.num())) return -1;

    QVector<Interval> matching_intervals = chr_intervals_[chr.num()].overlappingIntervals(start,end,true);
    if (matching_intervals.size() > 0)
    {
        return matching_intervals[0].value;
    }
    return -1;
}


#endif // CHROMOSOMALINDEX_H
