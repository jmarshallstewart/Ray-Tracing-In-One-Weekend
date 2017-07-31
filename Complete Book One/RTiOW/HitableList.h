#pragma once
#include "Hitable.h"

class HitableList : public Hitable
{
public:
    HitableList() {}
    HitableList(Hitable** ppList, int n) :
        list(ppList),
        listSize(n)
    {}

    bool hit(const Ray& r, float t_min, float t_max, HitRecord& record) const;

    Hitable** list;
    int listSize;
};

bool HitableList::hit(const Ray& r, float t_min, float t_max, HitRecord& record) const
{
    HitRecord tempRecord;
    bool hitAnything = false;

    float closestSoFar = t_max;

    for (int i = 0; i < listSize; ++i)
    {
        if (list[i]->hit(r, t_min, closestSoFar, tempRecord))
        {
            hitAnything = true;
            closestSoFar = tempRecord.t;
            record = tempRecord;
        }
    }

    return hitAnything;
}