#ifndef TRANSPORT_STREAM_HPP_ENTRY
#define TRANSPORT_STREAM_HPP_ENTRY

#include "StreamSections.hpp"


typedef unsigned char ts_byte;


class TS_Packet {
public:
    PacketHeader *ph;
    PSI *psi;
    PAT *pat;
    PMT *pmt;
public:
    TS_Packet();
    ~TS_Packet();
public:
    void PrintHeader() const;
    PACKRES FindPAT(std::stringstream& ss,
                 std::set<unsigned long>& programs_PIDs);
    void FindPMT(std::stringstream& ss,
                 std::set<unsigned long>& programs_PIDs,
                 std::set<unsigned long>& es_set);
};
#endif
