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

namespace org { namespace sil { namespace graphite { namespace v2 {

struct Rule {
  ~Rule();
  const vm::Code * constraint, 
                 * action;
  unsigned short   sort;
  byte             preContext;
};

inline Rule::~Rule()
{
  delete constraint;
  delete action;
}


struct RuleEntry
{
  const Rule   * rule;

  inline bool operator < (const RuleEntry &r) const
  { 
    const unsigned short lsort = rule->sort, rsort = r.rule->sort; 
    return lsort > rsort || (lsort == rsort && rule < r.rule);
  }
  
  inline bool operator == (const RuleEntry &r) const
  {
    return rule == r.rule;
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
  return (rules != NULL);
}

inline bool State::is_transition() const
{
  return (transitions != NULL);
}


class SlotMap
{
public:
  enum {MAX_SLOTS=64};
  SlotMap(GrSegment & seg);
  
  Slot       * * begin();
  Slot       * * end();
  size_t         size() const;
  unsigned short context() const;
  void           setContext(unsigned short);
  
  Slot * const & operator[](int n) const;
  Slot       * & operator [] (int);
  void           pushSlot(Slot * const slot);

  GrSegment &    segment;
private:
  Slot         * m_slot_map[MAX_SLOTS+1];
  unsigned short m_size;
  unsigned short m_precontext;
};


class FiniteStateMachine
{
public:
  enum {MAX_RULES=64};

private:
  class Rules
  {
  public:
      Rules();
      void              clear();
      const RuleEntry * begin() const;
      const RuleEntry * end() const;
      size_t            size() const;
      
      void accumulate_rules(const State &state);

  private:
      RuleEntry * m_begin, 
                * m_end,
                  m_rules[MAX_RULES*2];
  };

public:
  FiniteStateMachine(SlotMap & map);
  void      setContext(short unsigned int);
  Rules     rules;
  SlotMap   & slots;
};

inline FiniteStateMachine::FiniteStateMachine(SlotMap& map)
: slots(map)
{
}

inline void FiniteStateMachine::setContext(short unsigned int ctxt)
{
  rules.clear();
  slots.setContext(ctxt);
}

inline FiniteStateMachine::Rules::Rules()
  : m_begin(m_rules)
{
  m_end = m_begin;
}

inline void FiniteStateMachine::Rules::clear() 
{
  m_end = m_begin;
}

inline const RuleEntry * FiniteStateMachine::Rules::begin() const
{
  return m_begin;
}

inline const RuleEntry * FiniteStateMachine::Rules::end() const
{
  return m_end;
}

inline size_t FiniteStateMachine::Rules::size() const
{
  return m_end - m_begin;
}

inline void FiniteStateMachine::Rules::accumulate_rules(const State &state)
{
  // Only bother if there are rules in the State object.
  if (state.size() == 0) return;
  
  // Merge the new sorted rules list into the current sorted result set.
  const RuleEntry * lre = begin(), * rre = state.rules;
  RuleEntry * out = m_rules + (m_begin == m_rules)*MAX_RULES;    
  m_begin = out; 
  while (lre != end())
  {
    if (*lre < *rre)      *out++ = *lre++;
    else if (*rre < *lre) { *out++ = *rre++; }
    else                { *out++ = *lre++; ++rre; }

    if (rre == state.rules_end) 
    { 
      while (lre != end()) { *out++ = *lre++; }
      m_end = out;
      return;
    }
  }
  while (rre != state.rules_end) { *out++ = *rre++; }
  m_end = out;
}

inline SlotMap::SlotMap(GrSegment & seg)
: m_size(0), m_precontext(0), segment(seg)
{
}

inline Slot * * SlotMap::begin()
{
  return &m_slot_map[0];
}

inline Slot * * SlotMap::end()
{
  return m_slot_map + m_size;
}

inline size_t SlotMap::size() const
{
  return m_size;
}

inline short unsigned int SlotMap::context() const
{
  return m_precontext;
}

inline void SlotMap::setContext(short unsigned int ctxt)
{
  m_size = 0;
  m_precontext = ctxt;
}

inline void SlotMap::pushSlot(Slot*const slot)
{
  m_slot_map[m_size++] = slot;
}

inline Slot * const & SlotMap::operator[](int n) const
{
  return m_slot_map[n];
}

inline Slot * & SlotMap::operator[](int n)
{
  return m_slot_map[n];
}

}}}}