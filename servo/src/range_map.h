#ifndef __RANGEMAP_H
#define __RANGEMAP_H

/* maps x in the range [a..b] to [c..d] */
template<typename sourceT, typename targetT>
targetT 
range_map(sourceT a, sourceT b, targetT c, targetT d, sourceT x)
{
  if(a == b)
    return 0;

  return c + (d - c)*((targetT)(x - a))/(b - a);
}

#endif // __RANGEMAP_H
