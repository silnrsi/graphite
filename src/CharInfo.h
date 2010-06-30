#pragma once

class CharInfo // : ICharInfo
{

public:
    void init(int cid, int gindex) { m_char = cid; m_before = m_after = gindex; }
    void update(int offset) { m_before += offset; m_after += offset; }
    void feats(int offset) { m_featureid = offset; }
    int fid() { return m_featureid; }
    int breakWeight() { return m_break; }
    void breakWeight(int val) { m_break = val; }

protected:
    int m_char;     // Unicode character in character stream
    int m_before;   // slot id of glyph that cursor before this char is before
    int m_after;    // slot id of glyph that cursor after this char is after
    uint8 m_featureid;	// index into features list in the segment
    int8 m_break;	// breakweight coming from lb table
};


