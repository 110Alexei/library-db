#pragma once
#include <vector>
#include <fstream>
#include <iterator>
#include <iostream>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>
#include "Record.hpp"
#include "Query.hpp"

typedef enum
{
    OK,
    NO_RES,
    FALSE
} Result;

struct ArrayRecord
{
    Record R_;
    size_t index_;

    ArrayRecord(const Record &R, size_t index) : R_(R), index_(index)
    {
    }
};

inline void sort_data(std::vector<ArrayRecord> &v)
{
    std::sort(v.begin(), v.end(), [](const ArrayRecord &a, const ArrayRecord &b)
              {
                  if (a.R_.surname != b.R_.surname)
                      return a.R_.surname < b.R_.surname;
                  if (a.R_.name != b.R_.name)
                      return a.R_.name < b.R_.name;
                  return a.R_.lastname < b.R_.lastname;
              });

    for (size_t i = 0; i < v.size(); i++)
        v[i].index_ = i;
}

class Base
{
private:
    std::vector<ArrayRecord> data;
    std::vector<size_t> indexes;

    std::unordered_multimap<std::string, size_t> cl_1;
    std::unordered_multimap<std::string, size_t> cl_2;
    std::unordered_multimap<std::string, size_t> cl_3;

    std::string fin_name;
    std::string fout_name;

    bool sel_flag = false;

    Base(const Base &other) = delete;
    Base &operator=(const Base &other) = delete;

    Result do_query(const Query &q);

    Result do_select(const Query &q);
    Result do_reselect(const Query &q);
    Result do_remove(const Query &q);
    Result do_insert(const Query &q);
    Result do_print(const Query &q);

    Result make_ind_list(const Query &q);
    Result do_search(const criteria &c);

    Result class_search(const criteria &c);

    Result surname_search(const criteria &c);
    Result surname_search_full(const criteria &c);
    Result surname_search_begin(const criteria &c);
    Result title_search(const criteria &c);
    Result publisher_search(const criteria &c);
    Result name_search(const criteria &c);
    Result name_search_full(const criteria &c);
    Result name_search_begin(const criteria &c);
    Result lastname_search(const criteria &c);
    Result lastname_search_full(const criteria &c);
    Result lastname_search_begin(const criteria &c);
    Result language_search(const criteria &c);
    Result circulation_search(const criteria &c);

    template <typename FieldAccessor>
    Result generic_full_search(const criteria &c, FieldAccessor field_of);

    template <typename FieldAccessor>
    Result generic_begin_search(const criteria &c, FieldAccessor field_of);

    template <typename Project>
    Result surname_binary_search_impl(const std::string &target, Project project);

    bool get_str(const std::string &str);

    bool remove(size_t ind);

    void rebuild_class_index();

    void print_sel(const Query &q) const;
    void print_specific_fields(const Record &R, Field F, bool last_flag);

public:
    Base() = default;
    Base(const std::string &fin, const std::string &fout);
    ~Base() = default;

    void get_file(const std::string &fin);
    void to_file(const std::string &fout);
    void to_file();
    Result get_query(const std::string &str);

    void print() const;
    void print_sel() const;
};

inline Base::Base(const std::string &fin, const std::string &fout)
{
    fout_name = fout;
    this->get_file(fin);
}

inline void Base::rebuild_class_index()
{
    cl_1.clear();
    cl_2.clear();
    cl_3.clear();

    for (size_t i = 0; i < data.size(); i++)
    {
        cl_1.emplace(data[i].R_.class_1, data[i].index_);
        if (!data[i].R_.class_2.empty())
            cl_2.emplace(data[i].R_.class_2, data[i].index_);
        if (!data[i].R_.class_3.empty())
            cl_3.emplace(data[i].R_.class_3, data[i].index_);
    }
}

inline void Base::get_file(const std::string &fin)
{
    std::string line;

    std::ifstream in(fin);
    if (in.is_open())
    {
        while (std::getline(in, line))
        {
            if (line.empty())
                continue;

            Record R;
            try
            {
                R = Record(line);
            }
            catch (const std::invalid_argument &e)
            {
                std::cerr << "Base::get_file: пропущена повреждённая строка: " << e.what() << std::endl;
                continue;
            }

            size_t ind = data.size();
            data.push_back(ArrayRecord(R, ind));
        }

        sort_data(data);
        rebuild_class_index();
    }

    fin_name = fin;

    if (fout_name == "")
        fout_name = fin;
}

inline void Base::to_file(const std::string &fout)
{
    std::ofstream of(fout, std::ofstream::out | std::ofstream::trunc);
    for (size_t i = 0; i < data.size(); i++)
    {
        if (i != 0)
            of << std::endl;
        of << data[i].R_;
    }
}

inline void Base::to_file()
{
    to_file(fout_name);
}

inline void Base::print() const
{
    for (size_t i = 0; i < data.size(); i++)
        std::cout << data[i].R_ << std::endl;
}

inline void Base::print_sel() const
{
    for (size_t i = 0; i < indexes.size(); i++)
        std::cout << data[indexes[i]].R_ << std::endl;
}

inline bool Base::remove(size_t ind)
{
    if (ind >= data.size())
        return false;

    data.erase(data.begin() + ind);

    return true;
}

inline bool Base::get_str(const std::string &str)
{
    Record R;
    try
    {
        R = Record(str);
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << "Base::get_str: некорректные данные для INSERT: " << e.what() << std::endl;
        return false;
    }

    size_t ind = data.size();
    data.push_back(ArrayRecord(R, ind));

    sort_data(data);
    rebuild_class_index();

    return true;
}

inline Result Base::get_query(const std::string &str)
{
    Query q(str);

    if (q.get_flag() == false)
        return FALSE;

    return do_query(q);
}

inline Result Base::do_query(const Query &q)
{
    switch (q.com)
    {
    case (SELECT):
        return do_select(q);
    case (RESELECT):
        return do_reselect(q);
    case (REMOVE):
        return do_remove(q);
    case (INSERT):
        return do_insert(q);
    case (PRINT):
        return do_print(q);
    default:
        return FALSE;
    }
}

inline Result Base::do_insert(const Query &q)
{
    indexes.clear();
    sel_flag = false;

    std::string str = q.cr[1].str_ + "; " + q.cr[4].str_ + "; " + q.cr[5].str_ +
                      "; " + q.cr[2].str_ + "; " + q.cr[0].str_ + "; " + q.cr[6].str_ + "; " + q.cr[3].str_ + "; " + q.cr[7].str_;

    if (!get_str(str))
        return FALSE;

    return OK;
}

inline Result Base::do_select(const Query &q)
{
    indexes.clear();
    Result res = make_ind_list(q);
    if (res == NO_RES)
        indexes.clear();
    sel_flag = true;
    return res;
}

inline Result Base::do_reselect(const Query &q)
{
    if (!sel_flag)
        return FALSE;
    Result res = make_ind_list(q);
    if (res == NO_RES)
    {
        sel_flag = false;
        indexes.clear();
    }
    return res;
}

inline Result Base::do_remove(const Query &q)
{
    indexes.clear();
    sel_flag = false;
    Result res = make_ind_list(q);
    if (res == NO_RES)
        indexes.clear();

    for (size_t i = 0; i < indexes.size(); i++)
        remove(indexes[i] - i);
    for (size_t i = 0; i < data.size(); i++)
        data[i].index_ = i;

    rebuild_class_index();

    return res;
}

inline void Base::print_specific_fields(const Record &R, Field F, bool last_flag)
{
    if (F == SURNAME)
        std::cout << R.surname;
    if (F == NAME)
        std::cout << R.name;
    if (F == LASTNAME)
        std::cout << R.lastname;
    if (F == TITLE)
        std::cout << '"' << R.title << '"';
    if (F == PUBLISHER)
        std::cout << '"' << R.publisher << '"';
    if (F == LANGUAGE)
        std::cout << R.language;
    if (F == CLASSIFICATION)
        std::cout << R.classification;
    if (F == CIRCULATION)
        std::cout << R.circulation;

    if (!last_flag)
        std::cout << "; ";
    else
        std::cout << std::endl;
}

inline Result Base::do_print(const Query &q)
{
    if (q.pr_fl == SELECTED)
    {
        if (!sel_flag)
            return FALSE;
        if (q.pr_fields.empty())
            print_sel();
        else
            for (size_t i = 0; i < indexes.size(); i++)
            {
                for (size_t j = 0; j < q.pr_fields.size() - 1; j++)
                    print_specific_fields(data[indexes[i]].R_, q.pr_fields[j], false);

                print_specific_fields(data[indexes[i]].R_, q.pr_fields[q.pr_fields.size() - 1], true);
            }
    }
    else
    {
        if (q.pr_fields.empty())
            print();
        else
            for (size_t i = 0; i < data.size(); i++)
            {
                for (size_t j = 0; j < q.pr_fields.size() - 1; j++)
                    print_specific_fields(data[i].R_, q.pr_fields[j], false);

                print_specific_fields(data[i].R_, q.pr_fields[q.pr_fields.size() - 1], true);
            }
    }

    return OK;
}

inline Result Base::make_ind_list(const Query &q)
{
    Result res = OK;
    for (size_t i = 0; i < q.cr.size(); i++)
    {
        res = do_search(q.cr[i]);
        if (res == NO_RES)
            return NO_RES;
    }
    return res;
}

inline Result Base::do_search(const criteria &c)
{
    switch (c.Field_)
    {
    case CLASSIFICATION:
        return class_search(c);
    case SURNAME:
        return surname_search(c);
    case TITLE:
        return title_search(c);
    case PUBLISHER:
        return publisher_search(c);
    case NAME:
        return name_search(c);
    case LASTNAME:
        return lastname_search(c);
    case LANGUAGE:
        return language_search(c);
    case CIRCULATION:
        return circulation_search(c);
    default:
        return NO_RES;
    }
}

inline Result Base::class_search(const criteria &c)
{
    std::string class_1 = "";
    std::string class_2 = "";
    std::string class_3 = "";
    class_decoding(c.str_, class_1, class_2, class_3);

    std::vector<size_t> ind_1;
    std::vector<size_t> ind_2;
    std::vector<size_t> ind_3;

    auto range1 = cl_1.equal_range(class_1);
    for (auto it = range1.first; it != range1.second; ++it)
        ind_1.push_back(it->second);
    if (ind_1.empty())
        return NO_RES;
    if (class_2 == "")
    {
        std::sort(ind_1.begin(), ind_1.end());
        indexes = std::move(ind_1);
        return OK;
    }

    auto range2 = cl_2.equal_range(class_2);
    for (auto it = range2.first; it != range2.second; ++it)
        ind_2.push_back(it->second);
    if (ind_2.empty())
        return NO_RES;
    if (class_3 == "")
    {
        std::sort(ind_1.begin(), ind_1.end());
        std::sort(ind_2.begin(), ind_2.end());
        indexes.clear();
        std::set_intersection(ind_1.begin(), ind_1.end(),
                              ind_2.begin(), ind_2.end(), std::back_inserter(indexes));
        if (indexes.empty())
            return NO_RES;
        return OK;
    }

    auto range3 = cl_3.equal_range(class_3);
    for (auto it = range3.first; it != range3.second; ++it)
        ind_3.push_back(it->second);
    if (ind_3.empty())
        return NO_RES;
    std::sort(ind_1.begin(), ind_1.end());
    std::sort(ind_2.begin(), ind_2.end());
    std::sort(ind_3.begin(), ind_3.end());
    indexes.clear();
    std::vector<size_t> intersection1;
    std::set_intersection(ind_1.begin(), ind_1.end(), ind_2.begin(), ind_2.end(),
                          std::back_inserter(intersection1));
    std::set_intersection(ind_3.begin(), ind_3.end(), intersection1.begin(), intersection1.end(),
                          std::back_inserter(indexes));
    if (indexes.empty())
        return NO_RES;
    return OK;
}

template <typename FieldAccessor>
inline Result Base::generic_full_search(const criteria &c, FieldAccessor field_of)
{
    if (indexes.empty())
        for (size_t i = 0; i < data.size(); i++)
            indexes.push_back(data[i].index_);

    std::vector<size_t> filter_ind;
    for (size_t i = 0; i < indexes.size(); i++)
        if (field_of(data[indexes[i]].R_) == c.str_)
            filter_ind.push_back(indexes[i]);

    if (filter_ind.empty())
        return NO_RES;

    std::vector<size_t> new_ind;
    std::set_intersection(filter_ind.begin(), filter_ind.end(),
                          indexes.begin(), indexes.end(), std::back_inserter(new_ind));
    if (new_ind.empty())
        return NO_RES;

    indexes = std::move(new_ind);
    return OK;
}

template <typename FieldAccessor>
inline Result Base::generic_begin_search(const criteria &c, FieldAccessor field_of)
{
    if (indexes.empty())
        for (size_t i = 0; i < data.size(); i++)
            indexes.push_back(data[i].index_);

    std::string c_str = c.str_.substr(0, c.str_.find('*'));

    std::vector<size_t> filter_ind;
    for (size_t i = 0; i < indexes.size(); i++)
        if (field_of(data[indexes[i]].R_).compare(0, c_str.size(), c_str) == 0)
            filter_ind.push_back(indexes[i]);

    if (filter_ind.empty())
        return NO_RES;

    std::vector<size_t> new_ind;
    std::set_intersection(filter_ind.begin(), filter_ind.end(),
                          indexes.begin(), indexes.end(), std::back_inserter(new_ind));
    if (new_ind.empty())
        return NO_RES;

    indexes = std::move(new_ind);
    return OK;
}

template <typename Project>
inline Result Base::surname_binary_search_impl(const std::string &target, Project project)
{
    if (indexes.empty())
        for (size_t i = 0; i < data.size(); i++)
            indexes.push_back(data[i].index_);

    if (indexes.empty())
        return NO_RES;

    bool found = false;
    size_t left = 0;
    size_t right = indexes.size() - 1;
    size_t middle = 0;

    while (left <= right)
    {
        middle = left + (right - left) / 2;
        std::string key = project(data[indexes[middle]].R_.surname);

        if (key == target)
        {
            found = true;
            break;
        }

        if (key > target)
        {
            if (middle == left)
                break;
            right = middle - 1;
        }
        else
        {
            left = middle + 1;
        }
    }

    if (!found)
        return NO_RES;

    std::vector<size_t> filter_ind;

    size_t lo = middle;
    while (lo != 0 && project(data[indexes[lo - 1]].R_.surname) == target)
        --lo;

    size_t hi = lo;
    while (hi < indexes.size() && project(data[indexes[hi]].R_.surname) == target)
    {
        filter_ind.push_back(indexes[hi]);
        ++hi;
    }

    std::vector<size_t> new_ind;
    std::set_intersection(filter_ind.begin(), filter_ind.end(),
                          indexes.begin(), indexes.end(), std::back_inserter(new_ind));
    if (new_ind.empty())
        return NO_RES;

    indexes = std::move(new_ind);
    return OK;
}

inline Result Base::surname_search(const criteria &c)
{
    if (c.Filter_ == FULL)
        return surname_search_full(c);
    if (c.Filter_ == BEGIN)
        return surname_search_begin(c);
    return NO_RES;
}

inline Result Base::surname_search_full(const criteria &c)
{
    return surname_binary_search_impl(c.str_, [](const std::string &s)
                                       { return s; });
}

inline Result Base::surname_search_begin(const criteria &c)
{
    std::string c_str = c.str_.substr(0, c.str_.find('*'));
    return surname_binary_search_impl(c_str, [&c_str](const std::string &s)
                                       { return s.substr(0, c_str.size()); });
}

inline Result Base::name_search(const criteria &c)
{
    if (c.Filter_ == FULL)
        return name_search_full(c);
    if (c.Filter_ == BEGIN)
        return name_search_begin(c);
    return NO_RES;
}

inline Result Base::name_search_full(const criteria &c)
{
    return generic_full_search(c, [](const Record &r) -> const std::string &
                                { return r.name; });
}

inline Result Base::name_search_begin(const criteria &c)
{
    return generic_begin_search(c, [](const Record &r) -> const std::string &
                                 { return r.name; });
}

inline Result Base::lastname_search(const criteria &c)
{
    if (c.Filter_ == FULL)
        return lastname_search_full(c);
    if (c.Filter_ == BEGIN)
        return lastname_search_begin(c);
    return NO_RES;
}

inline Result Base::lastname_search_begin(const criteria &c)
{
    return generic_begin_search(c, [](const Record &r) -> const std::string &
                                 { return r.lastname; });
}

inline Result Base::lastname_search_full(const criteria &c)
{
    return generic_full_search(c, [](const Record &r) -> const std::string &
                                { return r.lastname; });
}

inline Result Base::title_search(const criteria &c)
{
    return generic_full_search(c, [](const Record &r) -> const std::string &
                                { return r.title; });
}

inline Result Base::publisher_search(const criteria &c)
{
    return generic_full_search(c, [](const Record &r) -> const std::string &
                                { return r.publisher; });
}

inline Result Base::language_search(const criteria &c)
{
    return generic_full_search(c, [](const Record &r) -> const std::string &
                                { return r.language; });
}

inline Result Base::circulation_search(const criteria &c)
{
    return generic_full_search(c, [](const Record &r) -> const std::string &
                                { return r.circulation; });
}