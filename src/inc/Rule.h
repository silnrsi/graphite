/*  GRAPHITE2 LICENSING

    Copyright 2011, SIL International
    All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should also have received a copy of the GNU Lesser General Public
    License along with this library in the file named "LICENSE".
    If not, write to the Free Software Foundation, 51 Franklin Street,
    Suite 500, Boston, MA 02110-1335, USA or visit their web page on the
    internet at http://www.fsf.org/licenses/lgpl.html.

Alternatively, the contents of this file may be used under the terms of the
Mozilla Public License (http://mozilla.org/MPL) or the GNU General Public
License, as published by the Free Software Foundation, either version 2
of the License or (at your option) any later version.
*/

#pragma once

#include "inc/Code.h"
#include "inc/SlotMap.h"

namespace graphite2 {

class json;

struct Rule {
  const vm::Machine::Code * constraint,
                 * action;
  unsigned short   sort;
  byte             preContext;
#ifndef NDEBUG
  uint16           rule_idx;
#endif

  Rule();
  ~Rule() {}

  CLASS_NEW_DELETE;

private:
  Rule(const Rule &);
  Rule & operator = (const Rule &);
};

inline
Rule::Rule()
: constraint(0),
  action(0),
  sort(0),
  preContext(0)
{
#ifndef NDEBUG
  rule_idx = 0;
#endif
}


struct RuleEntry
{
  const Rule   * rule;

  inline
  bool operator < (const RuleEntry &r) const
  {
    const unsigned short lsort = rule->sort, rsort = r.rule->sort;
    return lsort > rsort || (lsort == rsort && rule < r.rule);
  }

  inline
  bool operator == (const RuleEntry &r) const
  {
    return rule == r.rule;
  }
};


struct State
{
  const RuleEntry     * rules,
                      * rules_end;

  bool   empty() const;
};

inline
bool State::empty() const
{
    return rules_end == rules;
}



class FiniteStateMachine
{
public:
  enum {MAX_RULES=128};

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
  FiniteStateMachine(SlotMap & map, json * logger);
  void      reset(SlotBuffer::iterator & slot, const short unsigned int max_pre_ctxt);

  Rules     rules;
  SlotMap   & slots;
  json    * const dbgout;
};


inline
FiniteStateMachine::FiniteStateMachine(SlotMap& map, json * logger)
: slots(map),
  dbgout(logger)
{
}

inline
void FiniteStateMachine::reset(SlotBuffer::iterator & slot, const short unsigned int max_pre_ctxt)
{
  rules.clear();
  int ctxt = 0;
  for (; ctxt != max_pre_ctxt && std::prev(slot); ++ctxt, --slot);
  slots.reset(*slot, ctxt);
}

inline
FiniteStateMachine::Rules::Rules()
  : m_begin(m_rules), m_end(m_rules)
{
}

inline
void FiniteStateMachine::Rules::clear()
{
  m_end = m_begin;
}

inline
const RuleEntry * FiniteStateMachine::Rules::begin() const
{
  return m_begin;
}

inline
const RuleEntry * FiniteStateMachine::Rules::end() const
{
  return m_end;
}

inline
size_t FiniteStateMachine::Rules::size() const
{
  return m_end - m_begin;
}

inline
void FiniteStateMachine::Rules::accumulate_rules(const State &state)
{
  // Only bother if there are rules in the State object.
  if (state.empty()) return;

  // Merge the new sorted rules list into the current sorted result set.
  const RuleEntry * lre = begin(), * rre = state.rules;
  RuleEntry * out = m_rules + (m_begin == m_rules)*MAX_RULES;
  const RuleEntry * const lrend = out + MAX_RULES,
                  * const rrend = state.rules_end;
  m_begin = out;
  while (lre != end() && out != lrend)
  {
    if (*lre < *rre)      *out++ = *lre++;
    else if (*rre < *lre) { *out++ = *rre++; }
    else                { *out++ = *lre++; ++rre; }

    if (rre == rrend)
    {
      while (lre != end() && out != lrend) { *out++ = *lre++; }
      m_end = out;
      return;
    }
  }
  while (rre != rrend && out != lrend) { *out++ = *rre++; }
  m_end = out;
}


} // namespace graphite2
