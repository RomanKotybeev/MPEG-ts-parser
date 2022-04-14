#ifndef TS_PARSER_HPP_ENTRY
#define TS_PARSER_HPP_ENTRY

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <sstream>
#include <bitset>
#include <unistd.h>
#include <fcntl.h>

#include "TS_Packet.hpp"


class TS_Parser {
protected:
    char *source;
    int fd;
    ts_byte buffer[PACKET_SIZE];
    char bin_repr[PACKET_SIZE * N_BITS_IN_BYTE];
    std::stringstream ss;
    TS_Packet *packet;
    std::set<unsigned long> programs_PIDs;
    std::set<unsigned long> es_set;
private:
    void BinaryRepresentation(int size);
public:
    TS_Parser(char *s) : source(s) {};
    virtual ~TS_Parser();
    virtual bool Init() = 0;
    int Parse();
};


class ParserFromFile : public TS_Parser {
public:
    ParserFromFile(char *a_source) : TS_Parser(a_source) {}
    ~ParserFromFile() = default;
public:
    virtual bool Init();
};


class ParserFromCast : public TS_Parser {
    sockaddr_in *addr;
    ip_mreq *group;
    char *ip_addr;
    int port;
public:
    ParserFromCast(char *a_source)
        : TS_Parser(a_source)
        , addr(new sockaddr_in)
        , group(new ip_mreq)
        {}
    ~ParserFromCast();
public:
    virtual bool Init();
    void ParseSource();
};
#endif
