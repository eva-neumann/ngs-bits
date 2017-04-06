#include "BedFile.h"
#include "Exceptions.h"
#include "ChromosomalIndex.h"
#include "Helper.h"
#include <QFile>
#include <QStringList>
#include <QTextStream>
#include "BasicStatistics.h"
#include "algorithm"
#include "cmath"
// EVA
# include "chrono"
using  ns = std::chrono::nanoseconds;
using get_time = std::chrono::steady_clock ;


BedLine::BedLine()
    : chr_()
    , start_(0)
    , end_(-1)
{
}

BedLine::BedLine(const Chromosome& chr, int start, int end, const QStringList& annotations)
    : chr_(chr)
    , start_(start)
    , end_(end)
    , annotations_(annotations)
{
}

bool BedLine::operator<(const BedLine& rhs) const
{
    if (chr_<rhs.chr_) return true;
    else if (chr_>rhs.chr_) return false;
    else if (start_==rhs.start_) return end_<rhs.end_;
    else return start_<rhs.start_;
}

BedLine BedLine::fromString(QString str)
{
    //normalize
    str = str.replace(',', ""); //remove thousands separator
    str = str.replace(':', '\t').replace('-', '\t'); //also accept "[c]:[s]-[e]"
    str = str.replace(QRegExp("[ ]+"), "\t"); //also accept "[c] [s] [e]" (with any number of spaces)

    //split
    QStringList parts = str.split('\t');
    if (parts.count()<3) return BedLine();

    //convert
    try
    {
        return BedLine(parts[0], Helper::toInt(parts[1]), Helper::toInt(parts[2]));
    }
    catch(...)
    {
        return BedLine();
    }
}

BedFile::BedFile()
{
}

void BedFile::append(const BedLine& line)
{
    //check input data
    if (!line.chr().isValid())
    {
        THROW(ArgumentException, "Invalid BED line chromosome - empty string!");
    }
    if (line.start()<1 || line.end()<1 || line.start()>line.end())
    {
        THROW(ArgumentException, "Invalid BED line range '" + QString::number(line.start()) + " to " + QString::number(line.end()) + "'!");
    }

    lines_.append(line);
}

long long BedFile::baseCount() const
{
    long long output = 0;
    for (int i=0; i<lines_.count(); ++i)
    {
        output += lines_[i].length();
    }
    return output;
}

QSet<Chromosome> BedFile::chromosomes() const
{
    QSet<Chromosome> output;
    for (int i=0; i<lines_.count(); ++i)
    {
        output.insert(lines_[i].chr());
    }
    return output;
}

void BedFile::load(QString filename)
{
    clear();

    //parse from stream
    QSharedPointer<QFile> file = Helper::openFileForReading(filename, true);
    while(!file->atEnd())
    {
        QByteArray line = file->readLine();
        while (line.endsWith('\n') || line.endsWith('\r')) line.chop(1);

        //skip empty lines
        if(line.length()==0) continue;

        //store headers
        if (line.startsWith("#") || line.startsWith("track ") || line.startsWith("browser "))
        {
            headers_.append(line);
            continue;
        }

        //error when less than 3 fields
        QList<QByteArray> fields = line.split('\t');
        if (fields.count()<3)
        {
            THROW(FileParseException, "BED file line with less than three fields found: '" + line.trimmed() + "'");
        }

        //error when chromosome is empty
        if (fields[0]=="")
        {
            THROW(FileParseException, "Empty BED file chromosome field '" + fields[0] + "'!");
        }

        //error on position converion
        int start_pos = Helper::toInt(fields[1], "start position", line);
        int end_pos = Helper::toInt(fields[2], "end position", line);

        //create line
        QStringList annos;
        for (int i=3; i<fields.count(); ++i)
        {
            annos.append(fields[i]);
        }
        append(BedLine(fields[0], start_pos+1, end_pos, annos));
    }
}

void BedFile::store(QString filename) const
{
    //open stream
    QSharedPointer<QFile> file = Helper::openFileForWriting(filename, true);
    QTextStream stream(file.data());

    //write headers
    foreach(const QByteArray& header, headers_)
    {
        stream << header.trimmed()  << "\n";
    }

    //write contents
    foreach(const BedLine& line, lines_)
    {
        QString line_text = line.chr().str() + "\t" + QString::number(line.start()-1) + "\t" + QString::number(line.end());
        stream << line_text.toLatin1();
        if (line.annotations().size()>0)
        {
            QString add_text = "\t" + line.annotations().join("\t");
            stream << add_text.toLatin1();
        }
        stream << "\n";
    }
}

QString BedFile::toText() const
{
    QString output;

    foreach(const BedLine& line, lines_)
    {
        output.append(line.chr().str() + "\t" + QString::number(line.start()-1) + "\t" + QString::number(line.end()));
        if (line.annotations().count()!=0)
        {
            output.append("\t" + line.annotations().join("\t"));
        }
        output.append("\n");
    }

    return output;
}

void BedFile::clearAnnotations()
{
    for(int i=0; i<lines_.count(); ++i)
    {
        lines_[i].annotations().clear();
    }
}

void BedFile::clearHeaders()
{
    headers_.clear();
}

void BedFile::sort(bool uniq)
{
    std::sort(lines_.begin(), lines_.end());

    //remove duplicates
    if (uniq)
    {
        lines_.erase(std::unique(lines_.begin(), lines_.end()), lines_.end());
    }
}

void BedFile::merge(bool merge_back_to_back, bool merge_names)
{
    //in the following code, we assume that at least one line is present...
    if (lines_.count()==0) return;

    //remove annotations data
    for(int i=0; i<lines_.count(); ++i)
    {
        if (merge_names)
        {
            QString name = lines_[i].annotations().count() ? lines_[i].annotations()[0] : "";
            lines_[i].annotations().clear();
            lines_[i].annotations().append(name);
        }
        else
        {
            lines_[i].annotations().clear();
        }
    }

    //sort if necessary
    if (!isSorted()) sort();

    //merge lines
    BedLine next_output_line = lines_.first();
    int next_output_index = 0;
    for (int i=1; i<lines_.count(); ++i)
    {
        const BedLine& line = lines_[i];

        if ( next_output_line.overlapsWith(line.chr(), line.start(), line.end())
             ||
             (merge_back_to_back && next_output_line.adjacentTo(line.chr(), line.start(), line.end()))
             )
        {
            if (line.end()>next_output_line.end())
            {
                next_output_line.setEnd(line.end());
                if (merge_names)
                {
                    next_output_line.annotations()[0] += "," + line.annotations()[0];
                }
            }
        }
        else
        {
            lines_[next_output_index] = next_output_line;
            ++next_output_index;
            next_output_line = line;
        }
    }

    //add last line
    lines_[next_output_index] = next_output_line;

    //remove excess lines
    lines_.resize(next_output_index+1);
}

void BedFile::extend(int n)
{
    if (n<1)
    {
        THROW(ArgumentException, "Cannot extend BED file by '" + QString::number(n) + "' bases!");
    }

    for (int i=0; i<lines_.count(); ++i)
    {
        BedLine& line = lines_[i];
        line.setStart(std::max(1, line.start() - n));
        line.setEnd(line.end() + n);
    }
}

void BedFile::shrink(int n)
{
    if (n<1)
    {
        THROW(ArgumentException, "Cannot shrink BED file by '" + QString::number(n) + "' bases!");
    }

    for (int i=0; i<lines_.count(); ++i)
    {
        BedLine& line = lines_[i];
        line.setStart(line.start() + n);
        line.setEnd(line.end() - n);
    }

    removeInvalidLines();
}

void BedFile::add(const BedFile& file2)
{
    for (int i=0; i<file2.count(); ++i)
    {
        append(file2[i]);
    }
}


void BedFile::subtract(const BedFile& file2)
{
    QTextStream outstream(stdout);
    QTime timer;
    QList<QString> timings;
outstream << "Huhu " << endl;
    timer.start();
    auto start = get_time::now(); //use auto keyword to minimize typing strokes :)
    ChromosomalIndex<BedFile> file2_idx(file2);
    auto end = get_time::now();
    auto diff = end - start;
    outstream << "Elapsed time is :  "<< std::chrono::duration_cast<ns>(diff).count()<<" ns "<< endl;
    timings.append("Interval tree   file 2: " + Helper::elapsedTime(timer));
    timer.restart();

    //remove annotations
//    clearAnnotations();
//    timings.append("Clear annotations: " + Helper::elapsedTime(timer));
//    timer.restart();

//    //subtract
//    int removed_lines = 0;
//    for (int i=0; i<lines_.count(); ++i)
//    {
//        int start_this = lines_[i].start();
//        int stop_this = lines_[i].end();
//        QVector<int> matches_file2 = file2_idx.matchingIndices(lines_[i].chr(), lines_[i].start(), lines_[i].end());
//        std::sort(matches_file2.begin(), matches_file2.end(), MinStartPositionContainer<BedFile>(*this));
//        //outstream << "BedFile::subtract matches_file2: " << endl;
//        timings.append("BedFile::subtract matches_file2: " + Helper::elapsedTime(timer));
//        timer.restart();
//        if (matches_file2.empty())
//        {
//            continue;
//        }

//        int start_other=file2[matches_file2[0]].start();
//        int stop_other=file2[matches_file2[0]].end();
//        bool append_line=false;
//        int j=1;

//        // subtract all (make region invalid)
//        if (start_other<=start_this && stop_other>=stop_this)
//        {
//            lines_[i].setStart(0);
//            lines_[i].setEnd(0);
//            ++removed_lines;
//            continue;
//        }
//        // proceed until either the whole current_merged interval is subtracted or all overlapping intervals are processed
//        while ((start_this < stop_this) && (j < matches_file2.count()))
//        {
//            // go ahead if the matching interval of other is covered by the current interval
//            if (file2[matches_file2[j]].end() <= stop_other)
//            {
//                ++j;
//                continue;
//            }
//            // merge the current and the next interval if they overlap
//            if (file2[matches_file2[j]].start() <= stop_other)
//            {
//                ++j;
//                stop_other = file2[matches_file2[j]].end();
//                continue;
//            }

//            // left part is remaining
//            if (start_other > start_this)
//            {
//                //create new region (left part)
//                if (!append_line)
//                {
//                    lines_[i].setStart(start_this);
//                    lines_[i].setEnd(start_other-1);
//                    append_line=true;
//                }
//                else
//                {
//                    append(BedLine(lines_[i].chr(), start_this, start_other-1));
//                }
//            }
//            start_this=stop_other;
//            start_other=file2[matches_file2[j]].start();
//            stop_other=file2[matches_file2[j]].end();
//            ++j;
//        }
//        // left part is remaining
//        if (start_other > start_this)
//        {
//            //create new region (left part)
//            if (!append_line)
//            {
//                lines_[i].setStart(start_this);
//                lines_[i].setEnd(start_other-1);
//                append_line=true;
//            }
//            else
//            {
//                append(BedLine(lines_[i].chr(), start_this, start_other-1));
//            }
//        }
//        // create new region for the remaining right part of the interval
//        if (stop_other < stop_this)
//        {
//            //create new region (left part)
//            if (!append_line)
//            {
//                lines_[i].setStart(stop_other+1);
//                lines_[i].setEnd(stop_this);
//                append_line=true;
//            }
//            else
//            {
//                append(BedLine(lines_[i].chr(), stop_other+1, stop_this));
//            }
//        }
//    }
//    outstream << "BedFile::subtract All subtracted: "<< endl;
//    timings.append("BedFile::subtract All subtracted: " + Helper::elapsedTime(timer));
//    timer.restart();
//    //remove invalid lines, if necessary
//    if (removed_lines!=0) removeInvalidLines();
//    timings.append("BedFile::subtract remove invalid lines: " + Helper::elapsedTime(timer));

//    foreach(const QString& line, timings)
//    {
//        outstream << line << endl;
//    }


}




/// ToDo Compare time! Remove foreach and reserve where possible
/// Profile again!
//void BedFile::subtract(const BedFile& file2)
//{
//    QTextStream outstream(stdout);
//    QTime timer;
//    QList<QString> timings;
//    timer.start();
//    ChromosomalIndex<BedFile> file1_idx(*this);
//    timings.append("BedFile::subtract Interval tree 1: " + Helper::elapsedTime(timer));
//    timer.restart();
//    ChromosomalIndex<BedFile> file2_idx(file2);
//    timings.append("BedFile::subtract Interval tree 2: " + Helper::elapsedTime(timer));

//    timer.restart();
//    lines_.clear();
//    timings.append("BedFile::subtract clear von file 1: " + Helper::elapsedTime(timer));

//    timer.restart();
//    QHash<Chromosome, IntervalTree<BedFile> >::const_iterator file1_tree_it = file1_idx.allIntervalTrees().begin();
//    while (file1_tree_it != file1_idx.allIntervalTrees().end())
//    {
//        Chromosome chromosome =file1_tree_it.key();
//        QVector<Interval> file1_remaining;
//        // if the chromosome is not part of file2 all intervals in file1 remain
//        file1_idx.subtract(chromosome, file2_idx,file1_remaining);
//        lines_.reserve(this->count()+file1_remaining.count());
//        QVector<Interval>::const_iterator it = file1_remaining.begin();
//        while (it != file1_remaining.end())
//        {
//            append(BedLine(chromosome, it->start(), it->end()));
//            ++it;
//        }

//        ++file1_tree_it;
//    }
//    timings.append("BedFile::subtract All subtracted: " + Helper::elapsedTime(timer));
//    foreach(const QString& line, timings)
//    {
//        outstream << line << endl;
//    }
//}

void BedFile::intersect(const BedFile& file2)
{
    ChromosomalIndex<BedFile> file2_idx(file2);

    //remove annotations and headers
    clearAnnotations();

    //intersect
    int lines_original = lines_.count();
    for (int i=0; i<lines_original; ++i)
    {
        const BedLine& line = lines_[i];
        QVector<int> matches = file2_idx.matchingIndices(line.chr(), line.start(), line.end());
        //not match => not intersect => remove
        if (matches.empty())
        {
            lines_[i].setStart(0);
            lines_[i].setEnd(0);
            continue;
        }

        //intersect with first region (update line)
        int start_original = line.start();
        int end_original = line.end();
        lines_[i].setStart(std::max(start_original, file2[matches[0]].start()));
        lines_[i].setEnd(std::min(end_original, file2[matches[0]].end()));

        //intersect with more regions (insert new lines => we must not use the 'line' variable inside the loop because the vector can be reallocated!)
        Chromosome chr_original = line.chr();
        for (int j=1; j<matches.count(); ++j)
        {
            lines_.append(BedLine(chr_original, std::max(start_original, file2[matches[j]].start()), std::min(end_original, file2[matches[j]].end()) ));
        }
    }

    removeInvalidLines();
}

void BedFile::overlapping(const BedFile& file2)
{
    //check target region is merged and create index
    if (!file2.isMerged())
    {
        THROW(ArgumentException, "Merged BED file required for calculating the overlap of BED files!");
    }
    ChromosomalIndex<BedFile> file2_idx(file2);

    //overlapping
    for (int i=0; i<lines_.count(); ++i)
    {
        if (file2_idx.matchingIndex(lines_[i].chr(), lines_[i].start(), lines_[i].end())==-1)
        {
            lines_[i].setStart(0);
            lines_[i].setEnd(0);
        }
    }

    removeInvalidLines();
}

void BedFile::chunk(int chunk_size)
{
    QVector<BedLine> new_lines;
    new_lines.reserve(lines_.count());

    for (int i=0; i<lines_.count(); ++i)
    {
        const BedLine& line = lines_[i];
        if (line.length()>chunk_size)
        {
            //determine how many chunks are optimal
            double length = line.length();
            int n = floor(length/chunk_size);
            if (fabs(chunk_size-(length/n)) > fabs(chunk_size-(length/(n+1))))
            {
                n += 1;
            }
            //calculate actual chunk sizes
            QVector<int> chunk_sizes(n, chunk_size);
            int rest = line.length()-n*chunk_size;
            int current_chunk = 0;
            while (rest!=0)
            {
                int sign = BasicStatistics::sign(rest);
                chunk_sizes[current_chunk] += sign;
                rest -= sign;
                ++current_chunk;
                if (current_chunk==n) current_chunk = 0;
            }
            //create new lines
            int start = line.start();
            BedLine new_line = line;
            for (int i=0; i<n; ++i)
            {
                int end = start+chunk_sizes[i]-1;
                new_line.setStart(start);
                new_line.setEnd(end);
                new_lines.append(new_line);
                start = end+1;
            }
        }
        else
        {
            new_lines.append(line);
        }
    }

    lines_.swap(new_lines);
}

void BedFile::removeInvalidLines()
{
    //shift valid lines to the front
    int o=0;
    for (int i=0; i<lines_.count(); ++i)
    {
        const BedLine& line = lines_[i];
        if (line.start()>0 && line.start()<=line.end())
        {
            if (i!=o) lines_[o] = line;
            ++o;
        }
    }

    //remove excess lines
    lines_.resize(o);
}

bool BedFile::isSorted() const
{
    for (int i=1; i<lines_.count(); ++i)
    {
        if (lines_[i]<lines_[i-1]) return false;
    }

    return true;
}

bool BedFile::isMerged() const
{
    if (isSorted())
    {
        for (int i=1; i<lines_.count(); ++i)
        {
            const BedLine& line = lines_[i];
            if (lines_[i-1].overlapsWith(line.chr(), line.start(), line.end()))
            {
                return false;
            }
        }
    }
    else
    {
        BedFile tmp = *this;
        tmp.sort();
        for (int i=1; i<tmp.count(); ++i)
        {
            const BedLine& line = tmp[i];
            if (tmp[i-1].overlapsWith(line.chr(), line.start(), line.end()))
            {
                return false;
            }
        }

    }

    return true;
}

bool BedFile::isMergedAndSorted() const
{
    for (int i=1; i<lines_.count(); ++i)
    {
        const BedLine& line = lines_[i];
        if (line<lines_[i-1])
        {
            return false;
        }
        if (lines_[i-1].overlapsWith(line.chr(), line.start(), line.end()))
        {
            return false;
        }
    }

    return true;
}

bool BedFile::overlapsWith(const Chromosome& chr, int start, int end) const
{
    for (int i=0; i<lines_.count(); ++i)
    {
        if (lines_[i].overlapsWith(chr, start, end))
        {
            return true;
        }
    }

    return false;
}
