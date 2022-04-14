#ifndef TRANSPORT_STREAM_HPP_ENTRY
#define TRANSPORT_STREAM_HPP_ENTRY

#include "StreamSections.hpp"


typedef unsigned char ts_byte;
typedef unsigned long ulong;


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
                    std::map<ulong, PAT*>& pat_map);
    void FindPMT(std::stringstream& ss,
                 std::map<ulong, PAT*>& pat_map,
                 std::set<ulong>& es_set);
};
#endif
