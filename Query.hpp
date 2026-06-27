#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#define STR_SIZE 1000

typedef enum
{
    SELECT,
    RESELECT,
    REMOVE,
    INSERT,
    PRINT
} Command;

typedef enum
{
    BEGIN,
    FULL,
    ERR
} Filter;

typedef enum
{
    CLASSIFICATION = 0,
    SURNAME = 1,
    TITLE = 2,
    PUBLISHER = 3,
    NAME = 4,
    LASTNAME = 5,
    LANGUAGE = 6,
    CIRCULATION = 7,
    ERROR = 8
} Field;

typedef enum
{
    ALL,
    SELECTED
} Print_Flag;

struct criteria
{
    Field Field_;
    Filter Filter_;
    std::string str_;
    criteria(){};
    criteria(Field F, Filter f, const std::string &str)
    {
        Field_ = F;
        Filter_ = f;
        str_ = str;
    }
};

inline bool comp(criteria &a, criteria &b)
{
    return a.Field_ < b.Field_;
}

class Query
{
    friend class Base;

private:
    bool cor_flag = true;
    Print_Flag pr_fl;
    std::vector<Field> pr_fields;
    Command com;
    std::vector<criteria> cr;

    std::vector<std::string> set_of_com = {"SELECT", "RESELECT", "REMOVE", "PRINT", "INSERT"};
    std::vector<std::string> set_of_fields = {"surname", "name", "lastname", "title",
                                              "classification", "language", "publisher", "circulation"};

    Query(const Query &other);
    Query operator=(const Query &other);

    bool check_cor(const std::string &str);
    bool insert(const std::string &str);
    bool print(const std::string &str);
    bool sel_res_rem(const std::string &str);

    Field check_field(const std::string &str);
    Filter check_filter(const std::string &str, const Field &f);
    Filter filter_type(const std::string &str);
    Filter check_class(const std::string &str);
    bool criteria_check();

    Command get_com(const std::string &str);

public:
    Query(){};
    Query(const std::string &str);
    void get_str(const std::string &str);
    bool get_flag() const;
};

inline Query::Query(const std::string &str)
{
    this->get_str(str);
}

inline void Query::get_str(const std::string &str)
{
    cor_flag = this->check_cor(str);
}

inline bool Query::check_cor(const std::string &s)
{
    if (s.size() == 0 || s.find(" ") == std::string::npos || s.find("; end;") == std::string::npos ||
        s.size() != s.find("; end;") + 6 || s.size() > STR_SIZE)
        return false;

    std::string c = "";
    size_t k = s.find(" ");

    for (size_t i = 0; i < k; i++)
        c.push_back(s[i]);

    if (std::find(begin(set_of_com), end(set_of_com), c) == end(set_of_com))
        return false;

    com = get_com(c);

    std::string new_s;
    for (size_t i = s.find(" ") + 1; i < s.size(); i++)
        new_s.push_back(s[i]);

    switch (com)
    {
    case (SELECT):
        return sel_res_rem(new_s);
    case (RESELECT):
        return sel_res_rem(new_s);
    case (REMOVE):
        return sel_res_rem(new_s);
    case (INSERT):
        return insert(new_s);
    case (PRINT):
        return print(new_s);
    default:
        return false;
    }
}

inline bool Query::insert(const std::string &str)
{
    if (str.find("surname") == std::string::npos || str.find("name") == std::string::npos ||
        str.find("lastname") == std::string::npos || str.find("title") == std::string::npos ||
        str.find("classification") == std::string::npos || str.find("language") == std::string::npos ||
        str.find("publisher") == std::string::npos || str.find("circulation") == std::string::npos)
        return false;
    size_t r = 0;
    while (r < str.find("; end;"))
    {
        if (str.find("=", r) == std::string::npos)
            return false;

        std::string field = "";

        for (size_t i = r; i < str.find("=", r); i++)
            field.push_back(str[i]);

        if (check_field(field) == ERROR)
            return false;

        Field F = check_field(field);

        r = str.find("=", r);

        if (str[r + 1] != '"' || str[r + 2] == '"' || str[str.find(";") - 1] != '"')
            return false;

        std::string filter = "";

        if (F == TITLE || F == PUBLISHER)
        {
            r++;
            for (size_t i = r; i < str.find(";", r); i++)
                filter.push_back(str[i]);
        }
        else
        {
            r = r + 2;
            for (size_t i = r; i < str.find('"', r); i++)
                filter.push_back(str[i]);

            if (F == CLASSIFICATION && check_filter(filter, F) == ERR)
                return false;
        }

        r = str.find(";", r);

        Filter f = FULL;

        criteria c(F, f, filter);
        cr.push_back(c);

        if (!criteria_check())
            return false;

        sort(cr.begin(), cr.end(), comp);

        if (str[r + 1] != ' ')
            return false;

        r = r + 2;
    }
    
    if(cr.size() != 8)
        return false;

    return true;
}

inline bool Query::print(const std::string &str)
{
    if ((str.find("all") == std::string::npos && str.find("selected") == std::string::npos) ||
        (str.find("all") != std::string::npos && str.find("selected") != std::string::npos))
        return false;

    size_t r = str.find(";");
    std::string first = "";
    if (str.find("all") != std::string::npos)
    {
        for (size_t i = 0; i < r; i++)
            first.push_back(str[i]);

        if (first != "all")
            return false;

        pr_fl = ALL;
    }
    else
    {
        for (size_t i = 0; i < r; i++)
            first.push_back(str[i]);

        if (first != "selected")
            return false;

        pr_fl = SELECTED;
    }
    while (r < str.find("; end;"))
    {
        if (str[r + 1] != ' ')
            return false;

        std::string field = "";

        for (size_t i = r + 2; i < str.find(";", r + 1); i++)
            field.push_back(str[i]);

        r = str.find(";", r + 1);

        if (check_field(field) == ERROR)
            return false;

        Field f = check_field(field);

        pr_fields.push_back(f);
    }

    return true;
}

inline bool Query::sel_res_rem(const std::string &str)
{
    if (str.find("=") == std::string::npos)
        return false;

    size_t r = 0;

    while (r < str.find("; end;"))
    {
        std::string field = "";
        std::string filter = "";

        for (size_t i = r; i < str.find("=", r); i++)
            field.push_back(str[i]);

        r = str.find("=", r);

        if (check_field(field) == ERROR)
            return false;

        Field F = check_field(field);

        if (str[r + 1] != '"' || str[r + 2] == '"' || str[r + 2] == ';')
            return false;

        size_t l = r + 1;

        r = str.find(";", r);

        if (str[r - 1] != '"')
            return false;

        for (size_t i = l + 1; i < r - 1; i++)
            filter.push_back(str[i]);

        if (check_filter(filter, F) == ERR)
            return false;

        Filter f = check_filter(filter, F);

        criteria c(F, f, filter);
        cr.push_back(c);

        if (!criteria_check())
            return false;

        sort(cr.begin(), cr.end(), comp);

        if (str[r + 1] != ' ')
            return false;

        r = r + 2;
    }

    return true;
}

inline bool Query::get_flag() const
{
    return cor_flag;
}

inline Command Query::get_com(const std::string &str)
{
    if (str == "SELECT")
        return SELECT;
    if (str == "RESELECT")
        return RESELECT;
    if (str == "REMOVE")
        return REMOVE;
    if (str == "INSERT")
        return INSERT;
    return PRINT;
}

inline Field Query::check_field(const std::string &str)
{
    if (std::find(begin(set_of_fields), end(set_of_fields), str) == end(set_of_fields))
        return ERROR;
    if (str == "surname")
        return SURNAME;
    if (str == "name")
        return NAME;
    if (str == "lastname")
        return LASTNAME;
    if (str == "title")
        return TITLE;
    if (str == "classification")
        return CLASSIFICATION;
    if (str == "language")
        return LANGUAGE;
    if (str == "publisher")
        return PUBLISHER;
    return CIRCULATION;
}

inline Filter Query::filter_type(const std::string &str)
{
    if (str.find("*") == std::string::npos)
        return FULL;
    else
    {
        if (str.size() == 1 || str.find("*") != str.size() - 1)
            return ERR;
        return BEGIN;
    }
}

inline Filter Query::check_filter(const std::string &str, const Field &f)
{
    if (f == CLASSIFICATION)
        return check_class(str);
    if (f == TITLE || f == PUBLISHER || f == CIRCULATION || f == LANGUAGE)
        return FULL;
    else
        return filter_type(str);
}

inline Filter Query::check_class(const std::string &str)
{
    if (str.find("/") == std::string::npos)
        return FULL;

    size_t r = str.find("/");

    if (str.size() == r + 1 || str[r + 1] == '/')
        return ERR;

    if (str.find("/", r + 1) == std::string::npos)
        return FULL;

    r = str.find("/", r + 1);

    if (str.size() == r + 1 || str[r + 1] == '/')
        return ERR;

    if (str.find("/", r + 1) != std::string::npos)
        return ERR;
    return FULL;
}

inline bool Query::criteria_check()
{
    if (cr.size() < 2)
        return true;
    if (cr.size() > 8)
        return false;

    for (size_t i = 0; i < cr.size() - 1; i++)
        for (size_t j = i + 1; j < cr.size(); j++)
            if (cr[i].Field_ == cr[j].Field_)
                return false;

    return true;
}