#pragma once
#include <iostream>
#include <string>
#include <stdexcept>
 
inline void class_decoding(const std::string &classification, std::string &class_1, std::string &class_2, std::string &class_3)
{
    size_t first = classification.find('/');
    if (first == std::string::npos)
    {
        class_1 = classification;
        class_2.clear();
        class_3.clear();
        return;
    }
 
    class_1 = classification.substr(0, first);
 
    size_t second = classification.find('/', first + 1);
    if (second == std::string::npos)
    {
        class_2 = classification.substr(first + 1);
        class_3.clear();
        return;
    }
 
    class_2 = classification.substr(first + 1, second - (first + 1));
    class_3 = classification.substr(second + 1);
}
 
struct Record
{
    std::string title;
 
    std::string surname;
    std::string name;
    std::string lastname;
 
    std::string classification;
    std::string class_1 = "";
    std::string class_2 = "";
    std::string class_3 = "";
 
    std::string language;
 
    std::string publisher;
 
    std::string circulation;
 
    Record() : title(),
               surname(),
               name(),
               lastname(),
               classification(),
               class_1(),
               class_2(),
               class_3(),
               language(),
               publisher(),
               circulation()
    {
    }

    Record(const std::string &str);

    Record(const Record &other) = default;
    Record(Record &&other) = default;
    Record &operator=(const Record &other) = default;
    Record &operator=(Record &&other) = default;
 
    bool operator==(const Record &other) const;
};
 
namespace record_detail
{
    inline size_t find_required(const std::string &str, const std::string &delim, size_t from, const char *what)
    {
        size_t pos = str.find(delim, from);
        if (pos == std::string::npos)
            throw std::invalid_argument(std::string("Record: malformed input (") + what + ")");
        return pos;
    }
}
 
inline Record::Record(const std::string &str)
{
    using record_detail::find_required;
    const std::string sep = "; ";
 
    size_t border = find_required(str, sep, 0, "missing separator after surname");
    surname = str.substr(0, border);
 
    size_t next = find_required(str, sep, border + sep.size(), "missing separator after name");
    name = str.substr(border + sep.size(), next - (border + sep.size()));
    border = next;
 
    next = find_required(str, sep, border + sep.size(), "missing separator after lastname");
    lastname = str.substr(border + sep.size(), next - (border + sep.size()));
    border = next;
 
    size_t title_start = border + sep.size();
    if (title_start >= str.size() || str[title_start] != '"')
        throw std::invalid_argument("Record: malformed input (title is not quoted)");
    next = find_required(str, sep, title_start + 1, "missing separator after title");
    if (next == 0 || str[next - 1] != '"')
        throw std::invalid_argument("Record: malformed input (title is not closed with a quote)");
    title = str.substr(title_start + 1, next - 1 - (title_start + 1));
    border = next;
 
    next = find_required(str, sep, border + sep.size(), "missing separator after classification");
    classification = str.substr(border + sep.size(), next - (border + sep.size()));
    border = next;
 
    class_decoding(classification, class_1, class_2, class_3);
 
    next = find_required(str, sep, border + sep.size(), "missing separator after language");
    language = str.substr(border + sep.size(), next - (border + sep.size()));
    border = next;
 
    size_t publisher_start = border + sep.size();
    if (publisher_start >= str.size() || str[publisher_start] != '"')
        throw std::invalid_argument("Record: malformed input (publisher is not quoted)");
    next = find_required(str, sep, publisher_start + 1, "missing separator after publisher");
    if (next == 0 || str[next - 1] != '"')
        throw std::invalid_argument("Record: malformed input (publisher is not closed with a quote)");
    publisher = str.substr(publisher_start + 1, next - 1 - (publisher_start + 1));
    border = next;
 
    circulation = str.substr(border + sep.size());
}
 
inline bool Record::operator==(const Record &other) const
{
    return title == other.title && surname == other.surname &&
           name == other.name && lastname == other.lastname &&
           class_1 == other.class_1 && class_2 == other.class_2 && class_3 == other.class_3 &&
           classification == other.classification &&
           language == other.language && publisher == other.publisher && circulation == other.circulation;
}
 
inline std::ostream &operator<<(std::ostream &s, const Record &R)
{
    s << R.surname << "; " << R.name << "; " << R.lastname
      << "; " << '"' << R.title << '"' << "; " << R.classification << "; "
      << R.language << "; " << '"' << R.publisher << '"' << "; " << R.circulation;
    return s;
}