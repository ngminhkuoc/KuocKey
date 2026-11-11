#include "Macro.h"
#include <windows.h>
#include <cstddef>
#include <cwchar>
#include <fstream>
#include <iterator>
#pragma comment(lib, "normaliz.lib")
#pragma comment(lib, "user32.lib")

namespace kuockey { namespace macro {

std::wstring fromUtf8(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring w(n > 0 ? n : 0, 0);
    if (n > 0) MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], n);
    return w;
}
std::string toUtf8(const std::wstring& w) {
    if (w.empty()) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(n > 0 ? n : 0, 0);
    if (n > 0) WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &s[0], n, nullptr, nullptr);
    return s;
}

std::wstring fold(const std::wstring& s) {
    std::wstring t = s;
    if (!s.empty()) {
        int n = NormalizeString(NormalizationC, s.c_str(), (int)s.size(), nullptr, 0);
        if (n > 0) {
            std::wstring c(n, 0);
            n = NormalizeString(NormalizationC, s.c_str(), (int)s.size(), &c[0], n);
            if (n > 0) { c.resize(n); t = c; }
        }
    }
    if (!t.empty()) CharLowerBuffW(&t[0], (DWORD)t.size());
    return t;
}

const wchar_t* checkKey(const std::wstring& key) {
    if (key.empty()) return L"Chưa nhập chữ gõ tắt.";
    if (fold(key).size() > MAX_KEY) return L"Chữ gõ tắt dài tối đa 30 ký tự.";
    for (wchar_t c : key)
        if (c <= L' ' || wcschr(L".,;:!?)\"", c)) return L"Chữ gõ tắt không được có dấu cách hay các dấu . , ; : ! ? ) \"";
    return nullptr;
}
const wchar_t* checkText(const std::wstring& text) {
    if (text.empty()) return L"Chưa nhập chữ thay thế.";
    if (text.size() > MAX_TEXT) return L"Chữ thay thế dài tối đa 1000 ký tự.";
    return nullptr;
}

// ---- table ---------------------------------------------------------------------------------------
void Table::reindex() {
    _idx.clear();
    for (size_t i = 0; i < _e.size(); i++) _idx[fold(_e[i].key)] = i;
}
const Entry* Table::find(const std::wstring& word) const {
    auto i = _idx.find(fold(word));
    return i == _idx.end() ? nullptr : &_e[i->second];
}
void Table::set(const std::wstring& key, const std::wstring& text) {
    std::wstring f = fold(key);
    auto i = _idx.find(f);
    if (i != _idx.end()) { _e[i->second].key = key; _e[i->second].text = text; return; }
    _e.push_back(Entry{key, text});
    _idx[f] = _e.size() - 1;
}
void Table::remove(const std::wstring& key) {
    auto i = _idx.find(fold(key));
    if (i == _idx.end()) return;
    _e.erase(_e.begin() + (std::ptrdiff_t)i->second);
    reindex();
}

// ---- CSV -----------------------------------------------------------------------------------------
namespace {

struct Field { std::string s; bool quoted = false; };

std::string trimAscii(const std::string& s) {
    size_t a = s.find_first_not_of(" \t");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t");
    return s.substr(a, b - a + 1);
}

void addRow(std::vector<Entry>& out, const std::vector<Field>& row, int* skipped) {
    if (row.size() == 1 && row[0].s.empty() && !row[0].quoted) return;   // an empty line
    bool ok = row.size() >= 2;
    std::wstring key, text;
    if (ok) {
        key = fromUtf8(row[0].quoted ? row[0].s : trimAscii(row[0].s));
        std::string t = row[1].s;
        if (!row[1].quoted) {   // no quotes: more commas belong to the text ("omw,On my way, sorry")
            for (size_t i = 2; i < row.size(); i++) t += "," + row[i].s;
            t = trimAscii(t);
        }
        text = fromUtf8(t);
        ok = !checkKey(key) && !checkText(text);
    }
    if (ok) out.push_back(Entry{key, text});
    else if (skipped) ++*skipped;
}

std::string csvField(const std::string& s) {
    bool q = s.find_first_of(",\"\r\n") != std::string::npos ||
             (!s.empty() && (s.front() == ' ' || s.front() == '\t' || s.back() == ' ' || s.back() == '\t'));
    if (!q) return s;
    std::string o = "\"";
    for (char c : s) { if (c == '"') o += "\"\""; else o += c; }
    return o + "\"";
}

} // namespace

std::vector<Entry> parseCsv(const std::string& in, int* skipped) {
    std::vector<Entry> out;
    if (skipped) *skipped = 0;
    size_t i = 0, n = in.size();
    if (n >= 3 && (unsigned char)in[0] == 0xEF && (unsigned char)in[1] == 0xBB && (unsigned char)in[2] == 0xBF) i = 3;
    std::vector<Field> row;
    Field f;
    bool inQuotes = false, any = false;   // any: something was read since the last line break
    auto endField = [&]() { row.push_back(f); f = Field(); };
    auto endRow = [&]() { endField(); addRow(out, row, skipped); row.clear(); any = false; };
    while (i < n) {
        char c = in[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < n && in[i + 1] == '"') { f.s += '"'; i += 2; continue; }
                inQuotes = false; i++; continue;
            }
            f.s += c; i++; continue;
        }
        any = true;
        if (c == '"' && f.s.empty() && !f.quoted) { inQuotes = true; f.quoted = true; i++; }
        else if (c == ',') { endField(); i++; }
        else if (c == '\r' || c == '\n') {
            if (c == '\r' && i + 1 < n && in[i + 1] == '\n') i++;
            i++;
            endRow();
        }
        else { f.s += c; i++; }
    }
    if (any || !f.s.empty() || f.quoted || !row.empty()) endRow();
    return out;
}

std::string toCsv(const std::vector<Entry>& entries, bool bom) {
    std::string o;
    if (bom) o = "\xEF\xBB\xBF";
    for (const Entry& e : entries) o += csvField(toUtf8(e.key)) + "," + csvField(toUtf8(e.text)) + "\r\n";
    return o;
}

// ---- files ---------------------------------------------------------------------------------------
bool readFile(const std::wstring& path, std::string& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    out.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    return true;
}
bool writeFile(const std::wstring& path, const std::string& data) {
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f) return false;
    f.write(data.data(), (std::streamsize)data.size());
    f.close();
    return !f.fail();
}

bool loadFile(const std::wstring& path, Table& t) {
    std::string raw;
    if (!readFile(path, raw)) return false;
    t.clear();
    for (const Entry& e : parseCsv(raw)) t.set(e.key, e.text);
    return true;
}
bool saveFile(const std::wstring& path, const Table& t) {
    std::wstring tmp = path + L".tmp";
    if (!writeFile(tmp, toCsv(t.all(), false))) { DeleteFileW(tmp.c_str()); return false; }
    if (!MoveFileExW(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) { DeleteFileW(tmp.c_str()); return false; }
    return true;
}

}} // namespace
