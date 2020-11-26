/*  GRAPHITE2 LICENSING

    Copyright 2010, SIL International
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

#include <cstring>
#include "graphite2/Segment.h"

#include "inc/Main.h"
#include "inc/Position.h"

namespace graphite2 {

typedef gr_attrCode attrCode;

class GlyphFace;
class Segment;
class Slot;
class ShapingContext;
class Font;

class Slot_data {
protected:
    constexpr Slot_data();
    Slot_data(Slot_data const &) = default;
    Slot_data(Slot_data &&) noexcept = default;
    
    Slot_data & operator = (Slot_data const &) = default;
    Slot_data & operator = (Slot_data &&) noexcept = default;

    Slot      * m_parent;   // index to parent we are attached to
    Slot      * m_child;    // index to first child slot that attaches to us
    Slot      * m_sibling;  // index to next child that attaches to our parent
    Position    m_position; // absolute position of glyph
    Position    m_shift;    // .shift slot attribute
    Position    m_advance;  // .advance slot attribute
    Position    m_attach;   // attachment point on us
    Position    m_with;     // attachment point position on parent
    float       m_just;     // Justification inserted space
    uint32      m_original; // charinfo that originated this slot (e.g. for feature values)
    uint32      m_before;   // charinfo index of before association
    uint32      m_after;    // charinfo index of after association
    uint32      m_index;    // slot index given to this slot during finalising
    uint16      m_glyphid;  // glyph id
    uint16      m_realglyphid;
    byte        m_attLevel;    // attachment level
    byte        m_bidiLevel;   // bidirectional level
    int8        m_bidiCls;     // bidirectional class
    struct {
        bool    deleted: 1,
                inserted: 1,
                copied: 1,
                positioned: 1,
                attached: 1,
                eol:1;
    }        m_flags;       // holds bit flags
    friend class Segment;
};

constexpr Slot_data::Slot_data()
: m_parent{nullptr}, m_child{nullptr}, m_sibling{0}, 
  m_just{0},
  m_original{0}, m_before{0}, m_after{0}, m_index{0},
  m_glyphid{0}, m_realglyphid{0},
  m_attLevel{0}, m_bidiLevel{0}, m_bidiCls{0},
  m_flags{false, false, false, false, false, false}
{}

class Slot : private Slot_data
{
    static constexpr int NUMJUSTPARAMS = 5;

    union attributes {
    private:
        struct {
            uint8_t size;
            int16_t data[sizeof(uintptr_t)/sizeof(int16_t)-1];
        }   local;
        struct { 
            uint8_t n_attrs, n_justs; 
            int16_t data[1]; 
        } * external;

        bool is_inline() const { return !external || uintptr_t(external) & 0x3;}

    public:
        attributes(size_t n_attrs, size_t n_justs = 0): external(nullptr) { reserve(n_attrs, n_justs); }
        attributes(attributes const & rhs): external{rhs.external} { operator = (rhs); }
        attributes(attributes && rhs) noexcept : external{rhs.external} { rhs.external = nullptr; }
        ~attributes() noexcept { if (!is_inline()) free(external); }

        void reserve(size_t target_num_attrs, size_t target_num_justs);
        attributes & operator = (attributes const & rhs);
        attributes & operator = (attributes && rhs) noexcept;

        size_t num_attrs() const { return is_inline() ? local.size : external->n_attrs + 1; }
        size_t num_justs() const { return is_inline() ? 0 : external->n_justs; }

        int16_t       * user_attributes() { return is_inline() ? local.data : external->data; }
        int16_t const * user_attributes() const { return is_inline() ? local.data : external->data; }
        int16_t       * just_info() { return is_inline() ? nullptr : external->data + external->n_attrs; }
        int16_t const * just_info() const { return is_inline() ? nullptr : external->data + external->n_attrs; }
    };

    bool has_justify() const { return m_attrs.num_justs() != 0; };
    void init_just_infos(Segment const & seg);

    attributes  m_attrs;
#if !defined GRAPHITE2_NTRACING
    uint8_t     m_gen;
#endif

public:
    struct iterator;

    Slot(size_t num_attrs = 0) : m_attrs{num_attrs} {}
    Slot(Slot const &);
    Slot(Slot &&) noexcept;
    Slot & operator=(Slot const &);
    Slot & operator=(Slot &&) noexcept;

    unsigned short gid() const { return m_glyphid; }
    Position origin() const { return m_position; }
    float advance() const { return m_advance.x; }
    void advance(Position &val) { m_advance = val; }
    Position advancePos() const { return m_advance; }
    int before() const { return m_before; }
    int after() const { return m_after; }
    uint32 index() const { return m_index; }
    void index(uint32 val) { m_index = val; }

    uint16 glyph() const { return m_realglyphid ? m_realglyphid : m_glyphid; }
    void setGlyph(Segment &seg, uint16 glyphid, const GlyphFace * theGlyph = NULL);
    void setRealGid(uint16 realGid) { m_realglyphid = realGid; }
    void adjKern(const Position &pos) { m_shift = m_shift + pos; m_advance = m_advance + pos; }
    void origin(const Position &pos) { m_position = pos + m_shift; }
    void originate(int ind) { m_original = ind; }
    int original() const { return m_original; }
    void before(int ind) { m_before = ind; }
    void after(int ind) { m_after = ind; }
    bool isBase() const { return (!m_parent); }
    void update(int numSlots, int numCharInfo, Position &relpos);
    Position finalise(const Segment & seg, const Font* font, Position & base, Rect & bbox, uint8 attrLevel, float & clusterMin, bool rtl, bool isFinal, int depth = 0);
    bool isDeleted() const { return m_flags.deleted; }
    void markDeleted(bool state) { m_flags.deleted = state; }
    bool isCopied() const { return m_flags.copied; }
    void markCopied(bool state) { m_flags.copied = state; }
    bool isPositioned() const { return m_flags.positioned; }
    void markPositioned(bool state) { m_flags.positioned = state; }
    bool isEndOfLine() const { return m_flags.eol; }
    void markEndOfLine(bool state) { m_flags.eol = state; }
    bool isInsertBefore() const { return !m_flags.inserted; }
    void markInsertBefore(bool state) { m_flags.inserted = !state; }
    uint8 getBidiLevel() const { return m_bidiLevel; }
    void setBidiLevel(uint8 level) { m_bidiLevel = level; }
    int8 getBidiClass(const Segment &seg);
    int8 getBidiClass() const { return m_bidiCls; }
    void setBidiClass(int8 cls) { m_bidiCls = cls; }
    int16 const *userAttrs() const { return m_attrs.user_attributes(); }
    // void userAttrs(int16 *p) { m_userAttr = p; }
    void setAttr(Segment & seg, attrCode ind, uint8 subindex, int16 val, const ShapingContext & map);
    int getAttr(const Segment &seg, attrCode ind, uint8 subindex) const;
    int getJustify(const Segment &seg, uint8 level, uint8 subindex) const;
    void setJustify(Segment &seg, uint8 level, uint8 subindex, int16 value);
    void attachTo(Slot *ap) { m_parent = ap; }
    Slot *attachedTo() const { return m_parent; }
    Position attachOffset() const { return m_attach - m_with; }
    Slot* firstChild() const { return m_child; }
    void firstChild(Slot *ap) { m_child = ap; }
    bool child(Slot *ap);
    Slot* nextSibling() const { return m_sibling; }
    void nextSibling(Slot *ap) { m_sibling = ap; }
    bool sibling(Slot *ap);
    bool removeChild(Slot *ap);
    int32 clusterMetric(Segment const & seg, uint8 metric, uint8 attrLevel, bool rtl) const;
    void positionShift(Position a) { m_position += a; }
    void floodShift(Position adj, int depth = 0);
    float just() const { return m_just; }
    void just(float j) { m_just = j; }
    Slot *nextInCluster(const Slot *s) const;
    bool isChildOf(const Slot *base) const;

#if !defined GRAPHITE2_NTRACING
    ~Slot() noexcept { ++m_gen; }
    uintptr_t generation() const { return m_gen; }
#endif
    CLASS_NEW_DELETE
};

inline
Slot::Slot(Slot && rhs) noexcept
: Slot_data(std::move(rhs)),
  m_attrs(std::move(rhs.m_attrs))
#if !defined GRAPHITE2_NTRACING
  ,m_gen(rhs.m_gen)
#endif
{
  m_parent = m_child = m_sibling = nullptr;
}


inline
Slot::Slot(Slot const & rhs) 
: Slot_data(rhs),
  m_attrs(rhs.m_attrs)
#if !defined GRAPHITE2_NTRACING
  ,m_gen(rhs.m_gen)
#endif
{
  m_parent = m_child = m_sibling = nullptr;
}

} // namespace graphite2
