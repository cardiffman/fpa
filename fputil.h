/*
 * fputil.h
 *
 *  Created on: May 10, 2018
 *      Author: Mike
 */

#ifndef FPUTIL_H_
#define FPUTIL_H_

#include <string>
#include <list>
#include <algorithm>

template <typename C>
std::string join(const C& parts, int joint) {
        std::string ret;
        if (parts.size()) {
                auto ppart = parts.begin();
                        ret += *ppart++;
                        for (; ppart != parts.end(); ++ppart) {
                                ret += joint;
                                ret += *ppart;
                        }
        }
        //cout << "Yeah " << ret << " Ok " << endl;
        return ret;
}

template <typename T> T append(T& first, const T& second) {
        first.insert(first.end(), second.begin(), second.end());
        return first;
}
template <typename T> T prepend(const T& first, T& second) {
                second.insert(second.begin(), first.begin(), first.end());
        return second;
}
template <typename T> T concat(const T& first, const T& second) {
        T r = first;
                r.insert(r.end(), second.begin(), second.end());
        return r;
}

template <typename T, typename UnaryOp, typename T2=std::string, typename C2=std::list<T2>>
C2 mapf(const T b, const T e, UnaryOp f)
{
        C2 out;
        std::transform(b, e, back_inserter(out), f);
        return out;
}
template <typename T, typename UnaryOp, typename T2=std::string, typename C2=std::list<T2>>
C2 mapf(const T* b, const T*e, UnaryOp f)
{
        C2 out;
        std::transform(b, e, back_inserter(out), f);
        return out;
}
template <typename T, typename UnaryOp, typename T2=std::string, typename T3=unsigned, typename C2=std::list<T2>>
C2 mapf(const T* a, const T3& n, UnaryOp f)
{
        C2 out;
        std::transform(a, a+n, back_inserter(out), f);
        return out;
}
template <typename T, typename UnaryOp, typename T2=std::string, typename T3=unsigned, typename C2=std::list<T2>>
C2 mapfl(const T* a, const T3& n, UnaryOp f)
{
        C2 out;
        for (T3 i=0; i!=n; ++i) {
                out.push_back(f(a[i],i));
        }
        return out;
}



#endif /* FPUTIL_H_ */
