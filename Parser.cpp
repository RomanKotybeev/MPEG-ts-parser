#include "Parser.hpp"

#define PORT 6666
#define GROUP_ADDR "225.1.1.1"
#define INTERFACE_ADDR "9.5.1.1"


TS_Parser::~TS_Parser()
{
    close(fd);
    if (packet) delete packet;
}

int TS_Parser::Parse() {
    int rc;
    packet = new TS_Packet();
    while (0 != (rc = read(fd, buffer, PACKET_SIZE))) {
        BinaryRepresentation(rc);
        packet->FindPAT(ss, programs_PIDs);
    }
    
    lseek(fd, 0, SEEK_SET); // To begining
    while (0 != (rc = read(fd, buffer, PACKET_SIZE))) {
        BinaryRepresentation(rc);
        packet->FindPMT(ss, programs_PIDs, es_set);
    }
    return 0;
}

void TS_Parser::BinaryRepresentation(int size)
{
    int bitidx = 0;
    for (int i = 0; i < size; i++) {
        ts_byte byte = buffer[i];
        for (int k = N_BITS_IN_BYTE - 1; k >= 0; k--) {
            bin_repr[bitidx] = (byte & (1 << k)) ? '1' : '0';
            bitidx++;
        }
    }
    ss = std::stringstream(bin_repr);
}


/******************************************
    Initialize if file is passing
******************************************/

bool ParserFromFile::Init()
{
    fd = open(source, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return false;
    }
    return true;
}

/******************************************
    Initialize if multicast is passing
******************************************/

bool ParserFromCast::Init()
{
    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd == -1) {
        perror("socket");
        return false;
    }
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr->sin_family = AF_INET;
    addr->sin_port = htons(PORT);
    addr->sin_family = htonl(INADDR_ANY);
    int res = bind(fd, (sockaddr *) &addr, sizeof(addr));
    if (res == -1) {
        perror("bind");
        return false;
    }

    group->imr_multiaddr.s_addr = inet_addr(source);
    group->imr_interface.s_addr = inet_addr(INADDR_ANY);
    res = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                     (char *)&group, sizeof(group));
    if (res == -1) {
        perror("multicast group");
        return false;
    }

    return true;
}

ParserFromCast::~ParserFromCast()
{
    if (addr) delete addr;
    if (group) delete group;
}
