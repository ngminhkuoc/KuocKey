// Bảng gõ tắt (shortcut table): short text that expands to longer text, like iOS text replacement.
// The table, its lookup and the CSV import/export, with no keyboard or window code, so the test
// (Windows/Tests/macrotest.cpp) can build it alone. Stored as UTF-8 CSV, one "shortcut,text" pair per line.
#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace kuockey { namespace macro {

/// A shortcut can't be longer than this (the engine's word buffer is 32), and the text it expands to is capped
/// so one expansion stays a reasonable amount of SendInput.
const size_t MAX_KEY = 30;
const size_t MAX_TEXT = 1000;

struct Entry { std::wstring key, text; };

/// The key as it is compared: Unicode-normalised (NFC) and lower case, so "OMW", "omw" and a decomposed "đ" all match.
std::wstring fold(const std::wstring& s);

/// Null when the shortcut can be used; otherwise why not (Vietnamese, for the settings window).
/// Spaces and . , ; : ! ? ) " end a word, so a shortcut holding one could never be matched.
const wchar_t* checkKey(const std::wstring& key);
const wchar_t* checkText(const std::wstring& text);

class Table {
public:
    const std::vector<Entry>& all() const { return _e; }
    bool empty() const { return _e.empty(); }
    size_t size() const { return _e.size(); }
    /// The entry for what was typed (case-insensitive), or null.
    const Entry* find(const std::wstring& word) const;
    /// Adds a shortcut, or replaces the text of one with the same key (ignoring case). The caller checks the key first.
    void set(const std::wstring& key, const std::wstring& text);
    void remove(const std::wstring& key);
    void clear() { _e.clear(); _idx.clear(); }
private:
    std::vector<Entry> _e;
    std::unordered_map<std::wstring, size_t> _idx;   // fold(key) -> index in _e
    void reindex();
};

std::wstring fromUtf8(const std::string& s);
std::string toUtf8(const std::wstring& w);

/// CSV (RFC 4180): "shortcut,text" per line; a field holding a comma, a quote or a line break is in quotes, with a
/// quote written twice. Reads UTF-8 with or without a BOM and LF or CRLF. A line with more than one comma and no
/// quotes is read as the shortcut and then everything after the first comma. Lines that can't be used (no comma,
/// a bad shortcut, no text) are counted in *skipped, not returned.
std::vector<Entry> parseCsv(const std::string& utf8, int* skipped = nullptr);
/// bom: a UTF-8 byte order mark first, so Excel opens Vietnamese text correctly (for files the user takes elsewhere).
std::string toCsv(const std::vector<Entry>& entries, bool bom);

/// false if the file doesn't exist or can't be read. Replaces the table's contents.
bool loadFile(const std::wstring& path, Table& t);
/// Writes to a temporary file, then replaces the target, so a crash never leaves half a table.
bool saveFile(const std::wstring& path, const Table& t);
bool readFile(const std::wstring& path, std::string& out);
bool writeFile(const std::wstring& path, const std::string& data);

}} // namespace
