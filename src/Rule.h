/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#pragma once

#include "Code.h"
#include <bits/stl_algo.h>

namespace org { namespace sil { namespace graphite { namespace v2 {

struct Rule {
  const vm::Code * constraint, 
                 * action;
  byte             preContext;
};


struct RuleEntry
{
  const Rule   * rule;
  unsigned short sort,
		 length;

  inline bool operator < (const RuleEntry &r) const
  { 
    return sort > r.sort || (sort == r.sort && rule < r.rule);
  }
  
  inline bool operator == (const RuleEntry &r) const
  {
    return rule == r.rule;
  }

  inline bool operator != (const RuleEntry &r) const
  {
    return rule != r.rule;
  }
};


struct State
{
  const RuleEntry     * rules,
		      * rules_end;
  const State * const * transitions;
  
  size_t size() const;
  bool   is_success() const;
  bool   is_transition() const;
};

inline size_t State::size() const 
{
  return rules_end - rules;
}

inline bool State::is_success() const
{
  return rules;
}

inline bool State::is_transition() const
{
  return transitions;
}


class FiniteStateMachine
{
public:
  enum {MAX_RULES=64};

private:
  class ResultSet
  {
  public:
      ResultSet(RuleEntry *begin=0, const RuleEntry * const end=0);
      void              clear();
      const RuleEntry * begin() const;
      const RuleEntry * end() const;
      size_t            size() const;
      
      void accumulate_rules(const State &state, const unsigned short length);
  private:
      RuleEntry	        m_rules[MAX_RULES*2];
      RuleEntry  *      m_begin;
      const RuleEntry * m_end;
  };
  
public:
  ResultSet  rules;
  Slot *     slotMap[MAX_RULES+1];
};


inline FiniteStateMachine::ResultSet::ResultSet(RuleEntry *first, const RuleEntry * const last)
  : m_begin(first ? first : m_rules)
{
  m_end = last ? last : m_begin;
}

inline void FiniteStateMachine::ResultSet::clear() 
{
  m_end = m_begin;
}

inline const RuleEntry * FiniteStateMachine::ResultSet::begin() const
{
  return m_begin;
}

inline const RuleEntry * FiniteStateMachine::ResultSet::end() const
{
  return m_end;
}

inline size_t FiniteStateMachine::ResultSet::size() const
{
  return m_end - m_begin;
}

inline void FiniteStateMachine::ResultSet::accumulate_rules(const State &state, const unsigned short length)
{
  // Only bother if there are rules in the State object.
  if (size() > 0 && state.size() > 0)
  {
    // Merge the new sorted rules list into the current sorted result set.
    RuleEntry * out = m_begin == m_rules ? m_rules + MAX_RULES : m_rules;    
    const RuleEntry * lre = begin(),
                    * rre = state.rules;
    m_begin = out; 
    while (lre != end() && rre != state.rules_end)
    {
      if (*lre < *rre)      *out++ = *lre++;
      else 
      {
	// We only want to add a rule if it's not already included.
	if (*lre != *rre)   
	{
	  *out = *rre; out->length = length;
	  ++out;
	}
	++rre;
      }
    }
    std::copy(lre, end(), out);
    out += end() - lre;
    for (; rre != state.rules_end; ++rre, ++out)
    {
      *out = *rre; out->length = length;
    }
    m_end = out;
  }
  else if (size() == 0)
  {
    // If the ResultSet is currently empty just copy the list into it
    RuleEntry * out = m_begin;
    for (const RuleEntry *rre = state.rules; rre != state.rules_end; ++rre, ++out)
    {
      *out = *rre; out->length = length;
    }
    m_end = out;
  }
}

}}}}