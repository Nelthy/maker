#pragma once

template<typename... DSs>
struct perm_hybrid {};

/*
  custom_ds{
    rank = 0;

    teplate<rank_t P>
    auto& get(){
      get<P>(m_data_sets);
    }

    data_type m_data;
    std::tuple<vec_ds<float, 1>, vec_ds<float, 1>> m_data_sets;
  }


  perm_hybrid<vec_ds<custom_ds>> p;

  auto& ds = std::get<0, 1>() // returns insert<0, 0, vec_ds<custom_ds>>;
  ds[0]    // ds.get<0>[0], type =  insert<0, custom_ds>()
  ds[0][0] // ds.get<0>[0].get<1>()[0], type = float.

  [a]<b>[c]<0>[d]

  A hybrid data set must have an [ikey<rank>] operator leading to its contained type.
  A hybrid data set must also have a get overload that has sizeof...Ps == rank - 1.

  hybrid_ds<perm_vec_ds<float, vec_ds<float>, r2_coherent<5, float>> p;
  p.get<1, 0>(0, 4, 1); // [0].get<1>()[4].get<0>()[1]
  p.get<1, 0>(0, 4, 1); // [0].get<1>().get<0>({4, 1})
  p.get<1, 0>(0, 4, 1); // [0].get<1>()[{4, 1}]

  hybrid_ds<perm_vec_ds<float, r2_coherent<5, float>>> ds;

  [0].get<0>().get<1>()[i]
  [pi]<p><c>[ci]

  This is two permutes for a rank 2 set breaking the rank = sizeof...(Ps) + 1 rule.

  This also has the permutes in an abritrary order. The +1 rule was made in reaction
  to the permutes having a defined order of [a]<b>[c]<d>[e], but with this...

  The only constraint we really need to keep is the one involving the index maps.

  A potential solution involves nested keys.
  [pi]<p, c>[ci]
  This would obey the rank+1 rule and provide a way to encode what permutes to go to.
  It would also make structures like the tri perm more natural.

  Nesting may not be needed.
    All nestings can be linearized to a single permute.

    we could just do [pi]<d>[ci] == <d>[pi][ci]

    coherent_perm<4, float> p;
    get<0>(p)[i]
    
    p is a permute that doesn't align it's sub permutes. 
    
    It has its permute before the index (<c>[ci]) instead of
    after like other permutes ([reg]<qt>)
    
    Something like this could work, and could be inserted into.
      [i]<c>[ci]
      vec_ds<coherent_perm<4, float>> p;
      p[i].get<c>()[ci]
      perm_inserter<1, vec_ds<coherent_perm<4, float>>> p2;
      get<c>()[{i, ci}]  

      This is generalizable to other permutes
      
      perm_inserter<1, vec_ds<perm<4, vec_ds<float>, vec_ds<double>>>> p3;
      get<c>()[i][ci]
      The perms in this situation don't align their inner data sets. They are 
      completly controlled by the outside inserter. 
*/
