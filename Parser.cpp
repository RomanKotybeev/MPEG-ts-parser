#include "Parser.hpp"


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
        PACKRES res = packet->FindPAT(ss, pat_map);
        if (res != PACKRES::SYNC_BYTE)
            lseek(fd, -PACKET_SIZE, SEEK_CUR);
    }

    PrintPAT();
    
    std::cout << "--------- PMT ---------\n";

    lseek(fd, 0, SEEK_SET); // To begining
    while (0 != (rc = read(fd, buffer, PACKET_SIZE))) {
        BinaryRepresentation(rc);
        packet->FindPMT(ss, pat_map, es_set);
    }

    std::cout << "Number of elementary streams: "
              << es_set.size() << '\n';

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

void TS_Parser::PrintPAT() const
{
    std::cout << "========= PAT =========\n";
    for (auto pat : pat_map)
        pat.second->Print();
    std::cout << "========= END =========\n\n";
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

    ParseSource();

    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
    int res = bind(fd, (sockaddr *)addr, sizeof(*addr));
    if (res == -1) {
        perror("bind");
        return false;
    }

    group->imr_multiaddr.s_addr = inet_addr(ip_addr);
    group->imr_interface.s_addr = htonl(INADDR_ANY);
    res = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                     (char *)group, sizeof(*group));
    if (res == -1) {
        perror("multicast group");
        return false;
    }

    return true;
}

void ParserFromCast::ParseSource()
{
    int i;
    int MAX_CHARS_IN_IPADDR = 16;
    ip_addr = new char[MAX_CHARS_IN_IPADDR];
    for (i = 0; *source != ':'; source++, i++)
        ip_addr[i] = *source;
    ip_addr[i] = '\0';
    source++;

    port = atoi(source);
}

ParserFromCast::~ParserFromCast()
{
    if (addr) delete addr;
    if (group) delete group;
}
