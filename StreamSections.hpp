#ifndef STREAM_SECTION_HPP
#define STREAM_SECTION_HPP

#include <iostream>
#include <sstream>
#include <bitset>
#include <set>
#include <vector>


enum {
    PACKET_SIZE = 188,
    BUF_MULTIPLIER = 21,
    N_BITS_IN_BYTE = 8,
    MAX_SECTION_LEN = 1021,
};

enum TABLE_ID {
    TABLE_PAT = 0x00,
    TABLE_PMT = 0x02,
};

enum ADAPT {
    RESERVED = 0b00,
    PAYLOAD_ONLY = 0b01,
    ADAPT_ONLY = 0b10,
    ADAPT_PAYLOAD = 0b11,
};

enum PACKRES {
    INVALID,
    NOT_TABLE,
    SYNC_BYTE = 0x47,
};

class StreamSection {
public:
    virtual ~StreamSection() = default;
    virtual bool Set(std::stringstream& ss) = 0;
    void SkipBytes(std::stringstream& ss, unsigned long len);
    void RevertBytes(std::stringstream& ss, unsigned long capacity);
};


// Header of the packet
class PacketHeader : public StreamSection {
public:
    std::bitset<8> sync_byte;
    std::bitset<1> transport_error_indicator;
    std::bitset<1> payload_unit_start_indicator;
    std::bitset<1> transport_priority;
    std::bitset<13> PID;
    std::bitset<2> transport_scrambling_control;
    std::bitset<2> adaptation_field_control;
    std::bitset<4> continuity_counter;
    std::bitset<8> adaptation_field_length;
    bool is_PAT;
    bool PID_in_range;
private:
    bool HasAdaptationField();
    bool HasPSI();
public:
    PacketHeader() = default;
    ~PacketHeader() = default;
public:
    virtual bool Set(std::stringstream& ss);

    bool IsPAT() const { return is_PAT; }
    bool IsInRange() const { return PID_in_range; }

    bool HasPayloadIndicator() const
        { return payload_unit_start_indicator == 1; }
    unsigned long GetCC() const
        { return continuity_counter.to_ulong(); }
    unsigned long GetPID() const { return PID.to_ulong(); }
    unsigned long GetSyncByte() const { return sync_byte.to_ulong(); }

    // Print all bits
    void Print() const;
};


// Program Specific Information
class PSI : public StreamSection {
protected:
    std::bitset<8> pointer_field;
    std::bitset<8> table_id;
    std::bitset<1> section_syntax_indicator;
    std::bitset<1> _zero;
    std::bitset<2> reserved;
    std::bitset<12> section_length;
public:
    PSI() = default;
    virtual ~PSI() = default;
public:
    virtual bool Set(std::stringstream& ss);
    virtual int GetProgramSectionSize() const
        { return section_length.size(); };
    void SetStartTable(std::stringstream& ss);
};


class PAT : public PSI {
    std::bitset<16> transport_stream_id;
    std::bitset<2> reserved0;
    std::bitset<5> version_number;
    std::bitset<1> current_next_indicator;
    std::bitset<8> section_number;
    std::bitset<8> last_section_number;

    std::bitset<16> program_number;
    std::bitset<3> reserved1;
    std::bitset<13> network_PID;
    std::bitset<13> program_map_PID;

    std::bitset<32> CRC_32;
public:
    virtual bool Set(std::stringstream& ss);
    virtual int GetProgramSectionSize() const;
    void PrintPrograms() const;
    void AddPrograms(std::stringstream& ss,
                     std::set<unsigned long>& programs_PIDs);
};


class PMT : public PSI {
    std::bitset<16> program_number;
    std::bitset<2> reserved0;
    std::bitset<5> version_number;
    std::bitset<1> current_next_indicator;
    std::bitset<8> section_number;
    std::bitset<8> last_section_number;
    std::bitset<3> reserved1;
    std::bitset<13> PCR_PID;
    std::bitset<4> reserved2;
    std::bitset<12> program_info_length;

    std::bitset<8> stream_type;
    std::bitset<3> reserved3;
    std::bitset<13> elementary_PID;
    std::bitset<4> reserved4;
    std::bitset<12> ES_info_length;

    std::bitset<32> CRC_32;
private:
    void Print() const;
public:
    virtual bool Set(std::stringstream& ss);
    virtual int GetProgramSectionSize() const;
    int GetNestedSectionSize() const;
    void PrintES_Info(std::stringstream& ss,
                      std::set<unsigned long>& programs_PIDs,
                      std::set<unsigned long>& es_set);
};
#endif
