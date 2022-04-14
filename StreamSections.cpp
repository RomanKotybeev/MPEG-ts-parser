#include "StreamSections.hpp"


void StreamSection::SkipBytes(std::stringstream& ss, ulong len)
{
    ulong offset = (ulong) ss.tellg() + len;
    ss.seekg(offset);
}

void StreamSection::RevertBytes(std::stringstream& ss,
                                ulong capacity)
{
    ulong offset = (ulong) ss.tellg() - capacity;
    ss.seekg(offset);
}

void PacketHeader::Print() const
{
    std::cout   << std::hex <<
        "SB="   << sync_byte.to_ulong() <<
        " E="   << transport_error_indicator.to_ulong() <<
        " PUS=" << payload_unit_start_indicator.to_ulong() <<
        " Pr="  << transport_priority.to_ulong() <<
        " PID=" << PID.to_ulong() <<
        " TSC=" << transport_scrambling_control.to_ulong() <<
        " AF="  << adaptation_field_control.to_ulong() <<
        " CC="  << continuity_counter.to_ulong() <<
    std::endl;
}

bool PacketHeader::HasAdaptationField()
{
    if (adaptation_field_control == 0b10 ||
        adaptation_field_control == 0b11)
        return true;
    return false;
}

bool PacketHeader::HasPSI()
{
    if (PID == 0) {
        is_PAT = true;
        PID_in_range = false;
        return true;
    }
    else if (PID.to_ulong() >= 0x0010 && PID.to_ulong() <= 0x1FFE) {
        is_PAT = false;
        PID_in_range = true;
        return true;
    }
    return false;
}

bool PacketHeader::Set(std::stringstream& ss)
{
    ss >> sync_byte;
    ss >> transport_error_indicator;
    ss >> payload_unit_start_indicator;
    ss >> transport_priority;
    ss >> PID;
    ss >> transport_scrambling_control;
    ss >> adaptation_field_control;
    ss >> continuity_counter;

    if (adaptation_field_control == ADAPT::ADAPT_ONLY ||
        adaptation_field_control == ADAPT::RESERVED)
        return false;
    
    if (adaptation_field_control == ADAPT::ADAPT_PAYLOAD) {
        ss >> adaptation_field_length;
        SkipBytes(ss, adaptation_field_length.to_ulong());
    }
    return HasPSI();
}

bool PSI::Set(std::stringstream& ss)
{
    ss >> pointer_field;
    if (pointer_field != 0)
        SkipBytes(ss, pointer_field.to_ulong());
    return true;
}

void PSI::SetStartTable(std::stringstream& ss)
{
    ss >> table_id;
    ss >> section_syntax_indicator;
    ss >> _zero;
    ss >> reserved;
    ss >> section_length;
}

PAT::PAT(const PAT *pat)
{
    transport_stream_id    = pat->transport_stream_id;
    reserved0              = pat->reserved0;
    version_number         = pat->version_number;
    current_next_indicator = pat->current_next_indicator;
    section_number         = pat->section_number;
    last_section_number    = pat->last_section_number; 
    program_number         = pat->program_number;
    program_map_PID        = pat->program_map_PID;
}

void PAT::CheckAndAdd(std::map<ulong, PAT*>& pat_map, ulong pmid)
{
    if (pat_map.find(pmid) == pat_map.end())
        pat_map.insert(std::pair<ulong, PAT*>{pmid, new PAT(this)});
}

int PAT::GetProgramSectionSize() const
{
    return section_length.to_ulong()
        - (transport_stream_id.size()
           + reserved0.size()
           + version_number.size()
           + current_next_indicator.size()
           + section_number.size()
           + last_section_number.size()
           + CRC_32.size()
           ) / 8;
}

bool PAT::Set(std::stringstream& ss)
{
    SetStartTable(ss);
    if (table_id != TABLE_ID::TABLE_PAT)
        return false;
    ss >> transport_stream_id;
    ss >> reserved0;
    ss >> version_number;
    ss >> current_next_indicator;
    ss >> section_number;
    ss >> last_section_number; 

    return true;
}

void PAT::AddPrograms(std::stringstream& ss,
                      std::map<ulong, PAT*>& pat_map)
{
    bool ok = Set(ss);
    if (!ok)
        return;

    int section_size = GetProgramSectionSize();
    for (; section_size > 0; section_size -= 4) {
        ss >> program_number;
        ss >> reserved1;
        if (program_number == 0) {
            ss >> network_PID;
        }
        else {
            ss >> program_map_PID;
            CheckAndAdd(pat_map, program_map_PID.to_ulong());
        }
    }
    ss >> CRC_32;
}

void PAT::Print() const
{
    std::cout << "Program number: "
              << program_number.to_ulong() << '\n';
    std::cout << "Section length: "
              << section_length.to_ulong() << '\n';
    std::cout << "Transport stream id: "
              << transport_stream_id.to_ulong() << '\n';
    std::cout << "Program map PID: "
              << program_map_PID.to_ulong() << '\n';
}

bool PMT::Set(std::stringstream& ss)
{
    SetStartTable(ss);
    if (table_id != TABLE_ID::TABLE_PMT)
        return false;
    ss >> program_number;
    ss >> reserved0;
    ss >> version_number;
    ss >> current_next_indicator;
    ss >> section_number;
    ss >> last_section_number;
    ss >> reserved1;
    ss >> PCR_PID;
    ss >> reserved2;
    ss >> program_info_length;

    ss >> CRC_32;

    return true;
}

void PMT::PrintES_Info(std::stringstream& ss,
                       std::set<ulong>& es_set)
{
    bool ok = Set(ss);
    if (!ok)
        return;

    int section_size = GetProgramSectionSize();
    for (; section_size > 0; section_size -= 5) {
        ss >> stream_type;
        ss >> reserved3;
        ss >> elementary_PID;
        ss >> reserved4;
        ss >> ES_info_length;

        auto es_search = es_set.find(elementary_PID.to_ulong());
        if (es_search == es_set.end()) {
            es_set.insert(elementary_PID.to_ulong());
            Print();
        }
    }
}

void PMT::Print() const
{
    std::cout << "Program number: "
              << program_number.to_ulong() << '\n';
    std::cout << "Elementary PID: "
              << elementary_PID.to_ulong() << '\n';
    std::cout << "ES info length: "
              << ES_info_length.to_ulong() << '\n';
    std::cout << "Stream Type: " << std::hex
              << stream_type.to_ulong() << '\n';
    std::cout << '\n' << std::dec;
}

int PMT::GetProgramSectionSize() const
{
    return section_length.to_ulong()
        - (program_number.size()
           + reserved0.size()
           + version_number.size()
           + current_next_indicator.size()
           + section_number.size()
           + last_section_number.size()
           + reserved1.size()
           + PCR_PID.size()
           + reserved2.size()
           + program_info_length.size()
           + CRC_32.size()
          ) / 8;
}

int PMT::GetNestedSectionSize() const
{
    return ES_info_length.to_ulong()
        + (stream_type.size()
           + reserved3.size()
           + elementary_PID.size()
           + reserved4.size()
           + ES_info_length.size()
          ) / 8;
}
