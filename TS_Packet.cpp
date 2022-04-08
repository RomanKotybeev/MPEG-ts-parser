#include "TS_Packet.hpp"


TS_Packet::TS_Packet()
{
    ph = new PacketHeader();
    psi = new PSI();
    pat = new PAT();
    pmt = new PMT();
}

TS_Packet::~TS_Packet()
{
    if (ph) delete ph;
    if (psi) delete psi;
    if (pat) delete pat;
    if (pmt) delete pmt;
}

void TS_Packet::FindPAT(std::stringstream& ss,
                        std::set<unsigned long>& programs_PIDs)
{
    bool ok = ph->Set(ss);
    if (!ok)
        return;
    if (ph->HasPayloadIndicator()) {
        psi->Set(ss);
    }
    if (ph->IsPAT())
        pat->AddPrograms(ss, programs_PIDs);
}

void TS_Packet::FindPMT(std::stringstream& ss,
                        std::set<unsigned long>& programs_PIDs,
                        std::set<unsigned long>& es_set)
{
    bool ok = ph->Set(ss);
    if (!ok)
        return;
    if (ph->HasPayloadIndicator()) {
        psi->Set(ss);
    }
    if (ph->IsInRange())
        pmt->PrintES_Info(ss, programs_PIDs, es_set);
}

void TS_Packet::PrintHeader() const
{
    ph->Print();
}
