// Tests for the shortcut table (Bảng gõ tắt): lookup, validation and the CSV import/export. No keyboard involved.
// Exit code = number of failures. Built by Windows/App/CMakeLists.txt as kuockey_macrotest, run by Windows/check.ps1.
#include "Macro.h"
#include <cstdio>
#include <string>

using namespace kuockey::macro;

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static std::vector<Entry> parse(const char* utf8, int* skipped = nullptr) { return parseCsv(utf8, skipped); }

int main() {
    // lookup ignores case and Unicode composition
    {
        Table t;
        t.set(L"omw", L"On my way!");
        t.set(L"đc", L"được");   // đc -> được
        CHECK(t.find(L"OMW") && t.find(L"OMW")->text == L"On my way!");
        CHECK(t.find(L"Omw") != nullptr);
        CHECK(t.find(L"ĐC") != nullptr);          // ĐC
        CHECK(t.find(L"d̂") == nullptr);          // d + combining circumflex is not đ
        CHECK(t.find(L"om") == nullptr);
        t.set(L"OMW", L"Be there soon");               // same key ignoring case: replaced, not added
        CHECK(t.size() == 2 && t.find(L"omw")->text == L"Be there soon");
        t.remove(L"Omw");
        CHECK(t.size() == 1 && t.find(L"omw") == nullptr && t.find(L"đc") != nullptr);
        // a decomposed key (e + combining acute) is found by the composed one the engine types
        Table u; u.set(L"é", L"x");
        CHECK(u.find(L"é") != nullptr);
    }
    // key and text checks
    CHECK(checkKey(L"omw") == nullptr);
    CHECK(checkKey(L"@@") == nullptr);
    CHECK(checkKey(L"ty/ktd") == nullptr);
    CHECK(checkKey(L"") != nullptr);
    CHECK(checkKey(L"a b") != nullptr);
    CHECK(checkKey(L"a,b") != nullptr);
    CHECK(checkKey(L"a.b") != nullptr);
    CHECK(checkKey(L"a?") != nullptr);
    CHECK(checkKey(L"a\"") != nullptr);
    CHECK(checkKey(std::wstring(30, L'a')) == nullptr);
    CHECK(checkKey(std::wstring(31, L'a')) != nullptr);
    CHECK(checkText(L"") != nullptr);
    CHECK(checkText(std::wstring(1000, L'a')) == nullptr);
    CHECK(checkText(std::wstring(1001, L'a')) != nullptr);

    // import: plain lines
    {
        int skipped = -1;
        auto e = parse("omw,On my way!\nbrb,Be right back\n", &skipped);
        CHECK(e.size() == 2 && skipped == 0);
        CHECK(e[0].key == L"omw" && e[0].text == L"On my way!");
        CHECK(e[1].key == L"brb" && e[1].text == L"Be right back");
    }
    // CRLF, no final newline, blank lines, BOM
    {
        int skipped = -1;
        auto e = parse("\xEF\xBB\xBF" "a,one\r\n\r\nb,two", &skipped);
        CHECK(e.size() == 2 && skipped == 0 && e[0].key == L"a" && e[1].text == L"two");
    }
    // Vietnamese
    {
        auto e = parse("\xC4\x91\x63,\xC4\x91\xC6\xB0\xE1\xBB\xA3\x63\n");   // đc,được
        CHECK(e.size() == 1 && e[0].key == L"đc" && e[0].text == L"được");
    }
    // quoted fields: comma, doubled quote, line break
    {
        auto e = parse("addr,\"12 High St, London\"\nq,\"say \"\"hi\"\"\"\nsig,\"Kind regards,\nQuoc\"\n");
        CHECK(e.size() == 3);
        CHECK(e[0].text == L"12 High St, London");
        CHECK(e[1].text == L"say \"hi\"");
        CHECK(e[2].text == L"Kind regards,\nQuoc");
    }
    // an unquoted line with more commas: the text is everything after the first comma
    {
        auto e = parse("sorry,Sorry, I am late\n");
        CHECK(e.size() == 1 && e[0].text == L"Sorry, I am late");
    }
    // spaces around unquoted fields are dropped, inside quotes they stay
    {
        auto e = parse(" a , one \nb,\" two \"\n");
        CHECK(e.size() == 2 && e[0].key == L"a" && e[0].text == L"one" && e[1].text == L" two ");
    }
    // lines that can't be used are skipped and counted
    {
        int skipped = 0;
        auto e = parse("good,ok\nnocomma\n,no key\nbad key,text\nempty,\nlast,fine", &skipped);
        CHECK(e.size() == 2 && skipped == 4 && e[0].key == L"good" && e[1].key == L"last");
    }
    // export then import gives the same table, including awkward text
    {
        std::vector<Entry> in = {
            {L"omw", L"On my way!"}, {L"addr", L"12 High St, London"}, {L"q", L"say \"hi\""},
            {L"sig", L"Kind regards,\nQuoc"}, {L"đc", L"được"}, {L"pad", L" two "}, {L"emoji", L"\U0001F600 ok"}};
        for (bool bom : {false, true}) {
            std::string csv = toCsv(in, bom);
            CHECK((csv.compare(0, 3, "\xEF\xBB\xBF") == 0) == bom);
            int skipped = -1;
            auto out = parseCsv(csv, &skipped);
            CHECK(skipped == 0 && out.size() == in.size());
            for (size_t i = 0; i < in.size() && i < out.size(); i++) CHECK(out[i].key == in[i].key && out[i].text == in[i].text);
        }
        // one pair per line when nothing needs quotes
        CHECK(toCsv({{L"a", L"b"}, {L"c", L"d"}}, false) == "a,b\r\nc,d\r\n");
    }

    printf(failures ? "%d FAILED\n" : "all macro tests passed\n", failures);
    return failures;
}
