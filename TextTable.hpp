#ifndef CHOC_TEXT_TABLE_HEADER_INCLUDED
#define CHOC_TEXT_TABLE_HEADER_INCLUDED

#include "StringUtils.hpp"

namespace choc::text
{

struct TextTable
{
	TextTable()  = default;
	~TextTable() = default;

	/// Appends a column to the current row.
	TextTable &operator<<(std::string_view text);

	/// Clears and resets the table
	void clear();

	/// Ends the current row, so that subsequent new columns will be added to a new row
	void newRow();

	/// Returns the current number of rows
	size_t getNumRows() const;

	/// Finds the number of columns (by looking for the row with the most columns)
	size_t getNumColumns() const;

	/// Returns a vector of strings for each each row, which will have been
	/// padded and formatted to align vertically. Each row will have the prefix
	/// and suffix strings attached, and the columnSeparator string added between
	/// each column.
	std::vector<std::string> getRows(std::string_view rowPrefix,
	                                 std::string_view columnSeparator,
	                                 std::string_view rowSuffix) const;

	/// Returns a string containing all the rows in the table.
	/// See getRows() for details about the format strings. This method simply
	/// concatenates all the strings for each row.
	std::string toString(std::string_view rowPrefix,
	                     std::string_view columnSeparator,
	                     std::string_view rowSuffix) const;

  private:
	//==============================================================================
	struct Row
	{
		std::vector<std::string> columns;

		void        addColumn(std::string_view text);
		std::string toString(std::string_view columnSeparator, const std::vector<size_t> widths) const;
	};

	std::vector<Row> rows;
	bool             startNewRow = true;

	std::vector<size_t> getColumnWidths() const;
};

//==============================================================================
//        _        _           _  _
//     __| |  ___ | |_   __ _ (_)| | ___
//    / _` | / _ \| __| / _` || || |/ __|
//   | (_| ||  __/| |_ | (_| || || |\__ \ _  _  _
//    \__,_| \___| \__| \__,_||_||_||___/(_)(_)(_)
//
//   Code beyond this point is implementation detail...
//
//==============================================================================

inline TextTable &TextTable::operator<<(std::string_view text)
{
	if (startNewRow)
	{
		startNewRow = false;
		rows.push_back({});
	}

	rows.back().addColumn(text);
	return *this;
}

inline void TextTable::newRow()
{
	if (startNewRow)
		rows.push_back({});
	else
		startNewRow = true;
}

inline void TextTable::clear()
{
	rows.clear();
	startNewRow = true;
}

inline size_t TextTable::getNumRows() const
{
	return rows.size();
}

inline size_t TextTable::getNumColumns() const
{
	size_t maxColummns = 0;

	for (auto &row : rows)
		maxColummns = std::max(maxColummns, row.columns.size());

	return maxColummns;
}

inline std::vector<std::string> TextTable::getRows(std::string_view rowPrefix,
                                                   std::string_view columnSeparator,
                                                   std::string_view rowSuffix) const
{
	std::vector<std::string> result;
	result.reserve(rows.size());
	auto widths = getColumnWidths();

	for (auto &row : rows)
		result.push_back(std::string(rowPrefix) + row.toString(columnSeparator, widths) + std::string(rowSuffix));

	return result;
}

inline std::string TextTable::toString(std::string_view rowPrefix,
                                       std::string_view columnSeparator,
                                       std::string_view rowSuffix) const
{
	std::string result;

	for (auto &row : getRows(rowPrefix, columnSeparator, rowSuffix))
		result += row;

	return result;
}

inline std::vector<size_t> TextTable::getColumnWidths() const
{
	std::vector<size_t> widths;
	widths.resize(getNumColumns());

	for (auto &row : rows)
		for (size_t i = 0; i < row.columns.size(); ++i)
			widths[i] = std::max(widths[i], row.columns[i].length());

	return widths;
}

inline void TextTable::Row::addColumn(std::string_view text)
{
	columns.emplace_back(text);
}

inline std::string TextTable::Row::toString(std::string_view columnSeparator, const std::vector<size_t> widths) const
{
	std::string result;
	size_t      index = 0;

	for (auto &width : widths)
	{
		if (index != 0)
			result += columnSeparator;

		std::string padded;

		if (index < columns.size())
			padded = columns[index];

		padded.resize(width, ' ');
		result += padded;
		++index;
	}

	return result;
}

}        // namespace choc::text

#endif
