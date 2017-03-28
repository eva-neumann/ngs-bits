#include <iostream>
#include "TestFramework.h"
#include "IntervalTree.h"

TEST_CLASS(IntervalTree_Test)
{
    Q_OBJECT
    private slots:

//    void overlappingRandom()
//    {
//        QVector<int> indices;
//        QVector<Interval> intervals;
//        QVector<Interval> queries;

//        // generate a test set of target intervals
//        for (int i = 0; i < 10; ++i)
//        {
//            int range = 100-0;
//            int start = 0 + range * ((double) qrand() / (double) (RAND_MAX + 1.0));
//            int s = start + 10 * ((double) qrand() / (double) (RAND_MAX + 1.0));
//            int stop = (s < 1000) ? s : 1000;
//            intervals.append(Interval(start,stop,i));
//            indices.append(i);

//        }
//        IntervalTree<QVector<Interval> > interval_tree(intervals,indices,16);


//        // and queries
//        for (int i = 0; i < 5; ++i)
//        {
//            int range = 100-0;
//            int start = 0 + range * ((double) qrand() / (double) (RAND_MAX + 1.0));
//            int s = start + 10 * ((double) qrand() / (double) (RAND_MAX + 1.0));
//            int stop = (s < 1000) ? s : 1000;
//            queries.append(Interval(start,stop));
//        }
//        queries.push_back(Interval(15,19));

//        foreach (const Interval& i, queries)
//        {
//            QVector<Interval> matches;
//            interval_tree.overlappingIntervals(i.start(), i.end(), matches);
//            std::cout << "Interval " << i.start() << ","<< i.end() << " has the following matches" << std::endl;
//            for (int j=0; j<matches.size();++j)
//            {
//                std::cout << matches[j].start() << " " << matches[j].end() << " " << matches[j].value() << std::endl;
//            }
//        }
//    }

    void overlappingInterval()
    {
        QVector<Interval> intervals;
        std::list<int> indices;
        QVector<Interval> queries;

        intervals.push_back(Interval(14,20));
        indices.push_back(0);
        intervals.push_back(Interval(27,32));
        indices.push_back(1);
        intervals.push_back(Interval(33,40));
        indices.push_back(2);
        intervals.push_back(Interval(36,41));
        indices.push_back(3);
        intervals.push_back(Interval(47,53));
        indices.push_back(4);
        intervals.push_back(Interval(63,70));
        indices.push_back(5);
        intervals.push_back(Interval(78,85));
        indices.push_back(6);
        intervals.push_back(Interval(84,87));
        indices.push_back(7);
        intervals.push_back(Interval(91,92));
        indices.push_back(8);
        intervals.push_back(Interval(95,104));
        indices.push_back(9);
        IntervalTree<QVector<Interval> > interval_tree(intervals,indices,0,0);

        queries.push_back(Interval(1,3));
        queries.push_back(Interval(13,21));
        queries.push_back(Interval(15,19));
        queries.push_back(Interval(12,13));
        queries.push_back(Interval(99,101));
        queries.push_back(Interval(15,19));

        foreach (const Interval& i, queries)
        {
            QVector<int> matches;
            interval_tree.overlappingIntervals(i.start(), i.end(),  matches);
            std::cout << "Interval " << i.start() << ","<< i.end() << " has the following matches" << std::endl;
            for (int j=0; j<matches.size();++j)
            {
                std::cout << intervals[matches[j]].start() << " " << intervals[matches[j]].end() << " " << matches[j] << std::endl;
            }
        }
    }

};


