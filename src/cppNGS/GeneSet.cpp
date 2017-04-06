#include "GeneSet.h"
#include "Helper.h"

GeneSet::GeneSet()
	: QList<QByteArray>()
{
}

QByteArray GeneSet::join(QByteArray sep) const
{
	QByteArray output;

	foreach(const QByteArray& gene, *this)
	{
		if (!output.isEmpty()) output += sep;
		output += gene;
	}

	return output;
}

void GeneSet::insert(const QByteArray& gene)
{
	QByteArray tmp = gene.trimmed().toUpper();
	if (tmp.isEmpty()) return;

	auto it = std::lower_bound(begin(), end(), tmp);
	if(it == end() || *it != tmp)
	{
		QList<QByteArray>::insert(it, tmp);
	}
}

bool GeneSet::contains(const QByteArray& gene) const
{
	QByteArray tmp = gene.trimmed().toUpper();
	QList<QByteArray>::const_iterator it = std::lower_bound(begin(), end(), tmp);
	return (it != end() && *it == tmp);
}

bool GeneSet::contains(const GeneSet& genes) const
{
	foreach(const QByteArray& gene, genes)
	{
		if (contains(gene)) return true;
	}

	return false;
}

void GeneSet::store(QString filename) const
{
	auto handle = Helper::openFileForWriting(filename);
	foreach(const QByteArray& gene, *this)
	{
		handle->write(gene + "\n");
	}
}

GeneSet GeneSet::createFromFile(QString filename)
{
	GeneSet output;

	auto handle = Helper::openFileForReading(filename);
	while (!handle->atEnd())
	{
		QByteArray line = handle->readLine();
		if (line.startsWith("#")) continue;
		output.insert(line);
	}

	return output;
}

GeneSet GeneSet::createFromText(const QByteArray& text)
{
	GeneSet output;

	QList<QByteArray> lines = text.split('\n');
	foreach(const QByteArray& line, lines)
	{
		if (line.startsWith("#")) continue;
		output.insert(line);
	}

	return output;
}

